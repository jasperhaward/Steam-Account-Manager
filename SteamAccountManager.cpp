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

LPCSTR STEAM_REG_PATH = "Software\\Valve\\Steam";
LPCSTR STEAM_REG_LOGIN_KEY = "AutoLoginUser";
LPCSTR STEAM_REG_PW_KEY = "RememberPassword";

class Config {
 public:
  string dir;
  vector<string> usernames;

  void addUsername(string u) {
    usernames.push_back(u);
    save();
  }

  void removeUsername(int i) {
    usernames.erase(usernames.begin() + i);
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
      string usernameStr = getParameter(str, "usernames");

      // GET USERNAMES
      istringstream iss(usernameStr);
      string line;
      while (getline(iss, line)) {
        addUsername(line);
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

    file << "[usernames]";
    for (string &username : usernames) {
      file << '\n' << username;
    }

    file.close();
  }
};

void setup(Config &config);
void options(Config &config);
void additionalOptions(Config &config);

void start(string path);
void terminate(string szName);
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

  string u;
  cout << "Enter username: ";
  cin >> u;

  config.addUsername(u);
}

void options(Config &config) {
  cout << "Enter 0 for additional options. \n";
  for (size_t i = 0; i < config.usernames.size(); ++i) {
    cout << i + 1 << ". " << config.usernames[i] << "\n";
  }
  cout << "Enter option: ";

  int input = getValidInput(0, config.usernames.size());

  if (input == 0) {
    cout << endl;
    additionalOptions(config);
    cout << endl;
    options(config);
  } else {
    string username = config.usernames[input - 1];

    terminate(STEAM_EXE);
    updateRegistry(username);

    string steamPath = config.dir + "\\" + STEAM_EXE;
    start(steamPath);
  }
}

void additionalOptions(Config &config) {
  cout << "Additional options: \n"
       << "1. Add new username \n"
       << "2. Delete existing username \n"
       << "3. Change 'steam.exe' directory \n"
       << "Select option: ";

  int option = getValidInput(1, 3);

  if (option == 1) {
    string u;
    cout << "Enter username: ";
    cin >> u;

    config.addUsername(u);
  } else if (option == 2) {
    cout << "Enter username to delete: ";
    int i = getValidInput(1, config.usernames.size());

    config.removeUsername(i - 1);
  } else if (option == 3) {
    string dir;
    cout << "Enter 'steam.exe' directory: ";
    getline(cin, dir);

    config.changeDirectory(dir);
  }
}

// clang-format off
void start(string path) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  wstring lpApplicationName(path.begin(), path.end());

  CreateProcess(
    lpApplicationName.c_str(), 
    NULL, 
    NULL, 
    NULL, 
    FALSE, 
    0, 
    NULL,
    NULL, 
    &si, 
    &pi
  );
}
// clang-format on

// clang-format off
void terminate(string szName) {
  wstring wszName(szName.begin(), szName.end());

  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 1);

  if (Process32First(snapshot, &entry) == TRUE) {
    while (Process32Next(snapshot, &entry) == TRUE) {
      if (wcscmp(entry.szExeFile, wszName.c_str()) == 0) {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
        BOOL result = TerminateProcess(hProcess, 1);
        
        CloseHandle(hProcess);
      }
    }
  }

  CloseHandle(snapshot);
}

void updateRegistry(string username) {
  HKEY hKey;

  char lpLoginValue[50];
  strncpy(lpLoginValue, username.c_str(), sizeof(lpLoginValue));
  DWORD lpPwValue = 1;

  RegOpenKeyExA(
    HKEY_CURRENT_USER, 
    STEAM_REG_PATH, 
    0, 
    KEY_SET_VALUE, 
    &hKey
  );

  RegSetValueExA(
    hKey, 
    STEAM_REG_LOGIN_KEY, 
    0, 
    REG_SZ, 
    (const BYTE *)&lpLoginValue, 
    sizeof(lpLoginValue)
  );
  
  RegSetValueExA(
    hKey, 
    STEAM_REG_PW_KEY, 
    0, 
    REG_DWORD, 
    (const BYTE *)&lpPwValue, 
    sizeof(lpPwValue)
  );

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