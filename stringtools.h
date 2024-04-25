#include <iostream>
#include <string>
#include <vector>

std::string& trim(std::string& s, char t) {
    if (s.empty()) {
        return s;
    }
    s.erase(0, s.find_first_not_of(t));
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

std::vector<std::string> splitStr(std::string s, std::string del) {
    std::vector<std::string> res;
    int end = s.find(del);
    while (end != -1) {
        res.emplace_back(s.substr(0, end));
        s.erase(s.begin(), s.begin() + end + 1);
        end = s.find(del);
    }
    res.emplace_back(s.substr(0, end));
    return res;
}
