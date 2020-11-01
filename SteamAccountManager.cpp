#define NOMINMAX
#define UNICODE

// clang-format off
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
// clang-format on

#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

const char *CONFIG_FILENAME = "config.dat";

class Account {
 public:
  string username;
  string password;

  Account(string u, string p) {
    username = u;
    password = p;
  }
};

string getParameter(ifstream &file, string param);
vector<Account> getAccounts(string accountsString);
int getValidInput(int min, int max);
void printOptions(vector<Account> &accounts);
void changePath(string &path);
void addAccount(vector<Account> &accounts);
void deleteAccount(vector<Account> &accounts);
void saveConfig(string &path, vector<Account> &accounts);

int main() {
  ifstream file(CONFIG_FILENAME);
  string path;
  vector<Account> accounts;

  if (file.is_open()) {
    string ps = getParameter(file, "path");
    path = ps.erase(ps.size() - 1);

    string as = getParameter(file, "accounts");
    accounts = getAccounts(as);

    file.close();

    printOptions(accounts);

    int input = getValidInput(0, accounts.size());

    if (input == 0) {
      cout << "\nAdditional options:" << endl;
      cout << "1. Add new account" << endl;
      cout << "2. Delete existing account" << endl;
      cout << "3. Change 'steam.exe' path" << endl;
      cout << "Select option: ";
      int option = getValidInput(1, 3);

      if (option == 1) {
        addAccount(accounts);
      } else if (option == 2) {
        deleteAccount(accounts);
      } else {
        changePath(path);
      }
      saveConfig(path, accounts);
    } else {
      Account account = accounts[input - 1];

      cout << '\n' << "Opening account: ";
      cout << account.username << endl;

      // TERMINATE STEAM.EXE
      wstring processName(L"steam.exe");
      const wchar_t *szName = processName.c_str();

      PROCESSENTRY32 entry;
      entry.dwSize = sizeof(PROCESSENTRY32);

      HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 1);

      if (Process32First(snapshot, &entry) == TRUE) {
        while (Process32Next(snapshot, &entry) == TRUE) {
          if (wcscmp(entry.szExeFile, szName) == 0) {
            HANDLE hProcess =
                OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

            // Do stuff..
            BOOL result = TerminateProcess(hProcess, 1);
            CloseHandle(hProcess);
          }
        }
      }

      CloseHandle(snapshot);

      // STEAM LOGIN
      wstring wpath(path.begin(), path.end());
      LPCWSTR lpFile = wpath.c_str();

      string lCommand = "-login " + account.username + " " + account.password;
      wstring wcommand(lCommand.begin(), lCommand.end());
      LPCWSTR lpParameters = wcommand.c_str();

      ShellExecute(NULL, NULL, lpFile, lpParameters, NULL, SW_SHOW);
    }
  } else {
    cout << "First time setup..." << endl;
    changePath(path);
    addAccount(accounts);

    saveConfig(path, accounts);
  }

  return 0;
}

string getParameter(ifstream &file, string param) {
  // Reset flags
  file.clear();
  file.seekg(0);

  string line, values;
  bool copy = false;

  while (getline(file, line)) {
    if (line == '[' + param + ']') {
      copy = true;
    } else if (line[0] == '[') {
      copy = false;
    } else if (copy) {
      values += line + '\n';
    }
  }

  return values;
}

vector<Account> getAccounts(string accountsStr) {
  vector<Account> accounts;
  istringstream accs(accountsStr);
  string line, username, password;

  while (getline(accs, line)) {
    istringstream ss(line);
    ss >> username >> password;

    Account account(username, password);
    accounts.push_back(account);
  }

  return accounts;
}

int getValidInput(int min, int max) {
  int i;

  while (!(cin >> i) || i < min || i > max) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input. Try again: ";
  }

  return i;
}

void printOptions(vector<Account> &accounts) {
  cout << "Enter 0 for additional options." << endl;

  for (size_t i = 0; i < accounts.size(); ++i) {
    cout << i + 1 << ". " << accounts[i].username << endl;
  }

  cout << "Select option: ";
}

void addAccount(vector<Account> &accounts) {
  string username, password;

  cout << "Enter account username: ";
  cin >> username;

  cout << "Enter account password: ";
  cin >> password;

  Account account(username, password);
  accounts.push_back(account);
}

void deleteAccount(vector<Account> &accounts) {
  cout << "Enter account to delete: ";

  int i = getValidInput(1, accounts.size());

  accounts.erase(accounts.begin() + (i - 1));
}

void changePath(string &path) {
  cout << "Enter new path: ";

  cin.ignore();
  getline(cin, path);
}

void saveConfig(string &path, vector<Account> &accounts) {
  ofstream file(CONFIG_FILENAME);

  file << "[path]" << endl;
  file << path << endl;

  file << "[accounts]";
  for (size_t i = 0; i < accounts.size(); ++i) {
    file << endl << accounts[i].username;
    file << ' ' << accounts[i].password;
  }

  file.close();
}