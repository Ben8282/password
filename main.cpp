#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

static std::string strip(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == delim) { out.push_back(cur); cur.clear(); }
        else cur += c;
    }
    out.push_back(cur);
    return out;
}

static std::string input() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}
static std::string input(const std::string& prompt) {
    std::cout << prompt;
    return input();
}

struct Match { int a, b, size; };

static Match findLongestMatch(const std::string& a, const std::string& b,
                              int alo, int ahi, int blo, int bhi) {
    std::map<char, std::vector<int>> b2j;
    for (int j = blo; j < bhi; ++j) b2j[b[j]].push_back(j);

    int besti = alo, bestj = blo, bestsize = 0;
    std::map<int, int> j2len;
    for (int i = alo; i < ahi; ++i) {
        std::map<int, int> newj2len;
        auto it = b2j.find(a[i]);
        if (it != b2j.end()) {
            for (int j : it->second) {
                if (j < blo) continue;
                if (j >= bhi) break;
                int k = 1;
                auto pit = j2len.find(j - 1);
                if (pit != j2len.end()) k = pit->second + 1;
                newj2len[j] = k;
                if (k > bestsize) {
                    besti = i - k + 1;
                    bestj = j - k + 1;
                    bestsize = k;
                }
            }
        }
        j2len = std::move(newj2len);
    }
    while (besti > alo && bestj > blo && a[besti - 1] == b[bestj - 1]) {
        --besti; --bestj; ++bestsize;
    }
    while (besti + bestsize < ahi && bestj + bestsize < bhi &&
           a[besti + bestsize] == b[bestj + bestsize]) {
        ++bestsize;
    }
    return {besti, bestj, bestsize};
}

static int matchCount(const std::string& a, const std::string& b,
                      int alo, int ahi, int blo, int bhi) {
    Match m = findLongestMatch(a, b, alo, ahi, blo, bhi);
    if (m.size == 0) return 0;
    return m.size
         + matchCount(a, b, alo, m.a, blo, m.b)
         + matchCount(a, b, m.a + m.size, ahi, m.b + m.size, bhi);
}

static double ratio(const std::string& a, const std::string& b) {
    int T = (int)a.size() + (int)b.size();
    if (T == 0) return 1.0;
    int M = matchCount(a, b, 0, (int)a.size(), 0, (int)b.size());
    return 2.0 * M / T;
}

static const char* FILE_NAME = "password.txt";

int main() {
    std::cout << "Welcome to our Password Manager Version 1\n";
    std::cout << "Would you like to add a password or view one?\n";

    int reset = 1;
    while (reset == 1) {
        std::cout << "Type 1 to add a password\n";
        std::cout << "Type 2 to view a password\n";

        std::string response = input("> ");

        if (response == "1") {
            bool exists = fs::exists(FILE_NAME);

            std::cout << "please input the name of the password you would like to put\n";
            std::string name = input();
            if (exists)
                std::cout << "please input the password you would like to add that will be saved with " << name << "\n";
            else
                std::cout << "please input the password you would like to add that will be saved with the name " << name << "\n";
            std::string password = input();

            int next_id = 1;
            if (exists) {
                std::ifstream rf(FILE_NAME);
                std::string line;
                while (std::getline(rf, line)) {
                    if (strip(line).empty()) continue;
                    auto parts = split(strip(line), '|');
                    next_id = std::stoi(parts[0]) + 1;
                }
            }

            std::string entry = std::to_string(next_id) + "|" + name + "|" + password + "\n";
            std::cout << entry;
            std::ofstream f(FILE_NAME, std::ios::app);
            f << entry;
            f.close();

            std::cout << "Adding a password...\n";
            std::cout << "would you like to add or view another password\n";
            std::cout << "if yes say 1 if no say 2 or another number\n";
            std::string resetq = input();
            reset = (resetq == "1") ? 1 : 0;
        }
        else if (response == "2") {
            if (!fs::exists(FILE_NAME)) {
                std::cout << "No passwords saved yet. Add one first.\n";
            } else {
                int go_again = 1;
                while (go_again == 1) {
                    std::cout << "what would you like to view type 1 if you would like to view a specific password or 2 if you would like to view all your passwords\n";
                    std::string awsr = input();

                    if (awsr == "1") {
                        int a = 1;
                        while (a == 1) {
                            std::cout << "would you like to input a number that is in order of which password you added or search for name of the password\n";
                            std::cout << "press 1 for search by order of adding (search by id) or 2 for name\n";
                            std::string password_or_name = input();

                            if (password_or_name == "1") {
                                std::cout << "enter ID\n";
                                std::string searchpassword = input();
                                std::ifstream rf(FILE_NAME);
                                std::string line;
                                bool found = false;
                                while (std::getline(rf, line)) {
                                    if (strip(line).empty()) continue;
                                    auto parts = split(strip(line), '|');
                                    if (parts[0] == searchpassword) {
                                        std::cout << "Name: " << parts[1] << "\n";
                                        std::cout << "Password: " << parts[2] << "\n";
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) std::cout << "No password found with that ID.\n";
                                a = 0;
                            }
                            else if (password_or_name == "2") {
                                std::cout << "enter name to search\n";
                                std::string search_name = toLower(input());
                                std::ifstream rf(FILE_NAME);
                                std::string line;
                                std::vector<std::tuple<double, std::string, std::string>> entries;
                                while (std::getline(rf, line)) {
                                    if (strip(line).empty()) continue;
                                    auto parts = split(strip(line), '|');
                                    std::string stored_name = toLower(parts[1]);
                                    double r = ratio(search_name, stored_name);
                                    if (stored_name.find(search_name) != std::string::npos || r >= 0.6)
                                        entries.emplace_back(r, parts[1], parts[2]);
                                }
                                std::sort(entries.begin(), entries.end(),
                                          [](const auto& x, const auto& y) {
                                              return std::get<0>(x) > std::get<0>(y);
                                          });
                                if (!entries.empty()) {
                                    std::cout << "Found " << entries.size() << " match(es):\n";
                                    for (auto& e : entries)
                                        std::cout << "  Name: " << std::get<1>(e)
                                                  << "  |  Password: " << std::get<2>(e) << "\n";
                                } else {
                                    std::cout << "No matches found.\n";
                                }
                                a = 0;
                            }
                            else {
                                a = 1;
                            }
                        }
                        go_again = 0;
                    }
                    else if (awsr == "2") {
                        std::ifstream rf(FILE_NAME);
                        std::vector<std::string> lines;
                        std::string line;
                        while (std::getline(rf, line))
                            if (!strip(line).empty()) lines.push_back(line);

                        if (lines.empty()) {
                            std::cout << "No passwords saved yet.\n";
                        } else {
                            std::cout << "\n--- All Passwords (" << lines.size() << " total) ---\n";
                            for (auto& l : lines) {
                                auto parts = split(strip(l), '|');
                                std::cout << "  ID: " << parts[0]
                                          << "  |  Name: " << parts[1]
                                          << "  |  Password: " << parts[2] << "\n";
                            }
                            std::cout << "-----------------------------------\n\n";
                        }
                        go_again = 0;
                    }
                    else {
                        std::cout << "invalid answer\n";
                    }
                }
                std::cout << "Viewing a password...\n";
            }
            reset = 0;
        }
        else {
            std::cout << "Please enter either 1 or 2.\n";
        }
    }
    return 0;
}
