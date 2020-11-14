#define NOMINMAX
#define UNICODE

// clang-format off
#include <windows.h>
#include <tlhelp32.h>
#include <winreg.h>
#include <shellapi.h>
// clang-format on

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

string CONFIG_FILENAME = "config.dat";
string STEAM_EXE = "steam.exe";
string STEAM_DEFAULT_DIR = "C:\\Program Files (x86)\\Steam";
string STEAM_USERS_CONFIG_FILE = "\\config\\loginusers.vdf";

LPCSTR STEAM_REG_PATH = "Software\\Valve\\Steam";
LPCSTR STEAM_REG_LOGIN_KEY = "AutoLoginUser";
LPCSTR STEAM_REG_PW_KEY = "RememberPassword";

class Account {
 public:
  string username;
  string password;

  Account(string u, string p) {
    username = u;
    password = p;
  }
};

class Config {
 public:
  string dir;
  vector<Account> accounts;

  void addAccount(string u, string p) {
    Account a(u, p);
    accounts.push_back(a);
    save();
  }

  void removeAccount(int i) {
    accounts.erase(accounts.begin() + i);
    save();
  }

  void changeDirectory(string d) {
    dir = d;
    save();
  }

  bool load() {
    ifstream file(filename);

    if (file.is_open()) {
      stringstream buffer;
      buffer << file.rdbuf();
      string str = buffer.str();

      dir = getParameter(str, "dir");
      string accStr = getParameter(str, "accounts");

      // GET ACCOUNTS
      istringstream accss(accStr);
      string line, u, p;
      while (getline(accss, line)) {
        istringstream ss(line);
        ss >> u >> p;
        addAccount(u, p);
      }

      file.close();
      return true;
    } else {
      return false;
    }
  }

  Config(string f) { filename = f.c_str(); }

 private:
  const char *filename;

  string getParameter(string &str, string param) {
    string startStr = "[" + param + "]\n";
    string endStr = "[";

    unsigned start = str.find(startStr) + startStr.size();
    unsigned end = str.find(endStr, start) - endStr.size();

    return str.substr(start, end - start);
  }

  void save() {
    ofstream file(filename);

    file << "[dir]" << '\n';
    file << dir << '\n';

    file << "[accounts]";
    for (auto &account : accounts) {
      file << '\n' << account.username << ' ' << account.password;
    }

    file.close();
  }
};

void setup(Config &config);
void options(Config &config);
void additionalOptions(Config &config);

bool checkIfExistingAccount(string username, string configPath);
void start(string path, bool existingAccount, Account Account);
void terminate(string name);
void updateRegistry(string username);

int getValidInput(int min, int max);

int main() {
  Config config(CONFIG_FILENAME);
  bool existingConfig = config.load();

  if (!existingConfig) {
    setup(config);
    cout << endl;
  }
  options(config);

  return 0;
}

void setup(Config &config) {
  cout << "First time setup... \n"
       << "Use default Steam directory: '" << STEAM_DEFAULT_DIR << "'? \n"
       << "1. Yes \n"
       << "0. No \n"
       << "Select option: ";

  int option = getValidInput(0, 1);

  string dir = STEAM_DEFAULT_DIR;
  if (option == 0) {
    cout << "Enter 'steam.exe' directory: ";
    getline(cin, dir);
  }
  config.changeDirectory(dir);

  string u, p;
  cout << "Enter account username: ";
  cin >> u;
  cout << "Enter account password: ";
  cin >> p;

  config.addAccount(u, p);
}

void options(Config &config) {
  cout << "Enter 0 for additional options. \n";
  for (size_t i = 0; i < config.accounts.size(); ++i) {
    cout << i + 1 << ". " << config.accounts[i].username << "\n";
  }
  cout << "Enter option: ";

  int input = getValidInput(0, config.accounts.size());

  if (input == 0) {
    cout << endl;
    additionalOptions(config);
    cout << endl;
    options(config);
  } else {
    Account account = config.accounts[input - 1];

    string configPath = config.dir + STEAM_USERS_CONFIG_FILE;
    bool existingAccount = checkIfExistingAccount(account.username, configPath);

    terminate(STEAM_EXE);
    updateRegistry(account.username);

    string steamPath = config.dir + "\\" + STEAM_EXE;
    start(steamPath, existingAccount, account);
  }
}

void additionalOptions(Config &config) {
  cout << "Additional options: \n"
       << "1. Add new account \n"
       << "2. Delete existing account \n"
       << "3. Change 'steam.exe' directory \n"
       << "Select option: ";

  int option = getValidInput(1, 3);

  if (option == 1) {
    string u, p;
    cout << "Enter account username: ";
    cin >> u;
    cout << "Enter account password: ";
    cin >> p;

    config.addAccount(u, p);
  } else if (option == 2) {
    cout << "Enter account to delete: ";
    int i = getValidInput(1, config.accounts.size());

    config.removeAccount(i - 1);
  } else if (option == 3) {
    string dir;
    cout << "Enter 'steam.exe' directory: ";
    getline(cin, dir);

    config.changeDirectory(dir);
  }
}

bool checkIfExistingAccount(string username, string configPath) {
  ifstream file(configPath.c_str());

  if (file.is_open()) {
    stringstream buffer;
    buffer << file.rdbuf();
    string str = buffer.str();

    if (str.find(username) != string::npos) {
      return true;
    } else {
      return false;
    }
  }
}

// clang-format off
void start(string path, bool existingAccount, Account account) {
  wstring lpFile(path.begin(), path.end());
  
  string params = "-login " + account.username + " " + account.password;
  wstring wParams(params.begin(), params.end());
  
  auto lpParams = (existingAccount) ? NULL : wParams.c_str();

  ShellExecute(NULL, NULL, lpFile.c_str(), lpParams, NULL, SW_SHOW);
}
// clang-format on

// clang-format off
void terminate(string name) {
  wstring wsName(name.begin(), name.end());
  const wchar_t *szName = wsName.c_str();

  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 1);

  if (Process32First(snapshot, &entry) == TRUE) {
    while (Process32Next(snapshot, &entry) == TRUE) {
      if (wcscmp(entry.szExeFile, szName) == 0) {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
        BOOL result = TerminateProcess(hProcess, 1);
        CloseHandle(hProcess);
      }
    }
  }

  CloseHandle(snapshot);
}
// clang-format on

// clang-format off
void updateRegistry(string username) {
  HKEY hKey;

  char lpLoginValue[50];
  strncpy(lpLoginValue, username.c_str(), sizeof(lpLoginValue));
  DWORD lpPwValue = 1;

  RegOpenKeyExA(HKEY_CURRENT_USER, STEAM_REG_PATH, 0, KEY_SET_VALUE, &hKey);
  RegSetValueExA(hKey, STEAM_REG_LOGIN_KEY, 0, REG_SZ, (const BYTE *)&lpLoginValue, sizeof(lpLoginValue));
  RegSetValueExA(hKey, STEAM_REG_PW_KEY, 0, REG_DWORD, (const BYTE *)&lpPwValue, sizeof(lpPwValue));

  RegCloseKey(hKey);
}
// clang-format on

int getValidInput(int min, int max) {
  int i;

  while (!(cin >> i) || i < min || i > max) {
    cin.clear();
    cin.ignore(INT_MAX, '\n');
    cout << "Invalid input. Try again: ";
  }
  cin.ignore(INT_MAX, '\n');

  return i;
}