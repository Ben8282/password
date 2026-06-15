#include <iostream>
using namespace std;
int main() {
    int attempt = 3;
    while (attempt > 0) {
        cout << "Enter Password You Have " << attempt << " Attempts Left" << endl;
        string password;
        cin >> password;
        if (password == "WEYW") {
            cout << "Access Granted" << endl;
            break;
            
        }
        else {
            attempt --;
            cout << "Wrong Password Try Again You have " << attempt << " Attempts Left" << endl;
        }
        if (attempt == 0) {
            cout << "To Many Attempts" << endl;
            break;
        }
        }
    
}
