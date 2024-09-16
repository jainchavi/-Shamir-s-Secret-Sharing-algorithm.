#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <stdexcept>

// Simple JSON parser
class JSON {
public:
    std::map<std::string, std::map<std::string, std::string>> data;

    void parse(std::istream& is) {
        std::string line;
        std::string current_key;
        while (std::getline(is, line)) {
            // Remove whitespace
            line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
            if (line.empty() || line[0] == '{' || line[0] == '}') continue;

            size_t colon_pos = line.find(":");
            if (colon_pos == std::string::npos) {
                continue;
            }

            std::string key = line.substr(0, colon_pos);
            key.erase(0, key.find_first_not_of("\""));
            key.erase(key.find_last_not_of("\"") + 1);

            std::string value = line.substr(colon_pos + 1);
            value.erase(0, value.find_first_not_of("\""));
            value.erase(value.find_last_not_of("\",") + 1);

            if (value == "{") {
                current_key = key;
            } else {
                if (current_key.empty()) {
                    current_key = "root";
                }
                data[current_key][key] = value;
            }
        }
    }
};

// Function to convert a string number from any base to decimal
int64_t convert_to_decimal(const std::string& value, int base) {
    int64_t result = 0;
    for (char c : value) {
        if (isdigit(c)) {
            result = result * base + (c - '0');
        } else if (isalpha(c)) {
            result = result * base + (tolower(c) - 'a' + 10);
        } else {
            throw std::runtime_error("Invalid character in number: " + std::string(1, c));
        }
        if (result < 0) {
            throw std::overflow_error("Number too large for int64_t");
        }
    }
    return result;
}

// Function to perform Lagrange interpolation and find the secret
int64_t find_secret(const std::vector<std::pair<int64_t, int64_t>>& points) {
    int64_t secret = 0;
    int n = points.size();

    for (int i = 0; i < n; i++) {
        int64_t term = points[i].second;
        for (int j = 0; j < n; j++) {
            if (i != j) {
                if (points[i].first == points[j].first) {
                    throw std::runtime_error("Duplicate x values in points");
                }
                term = term * (-points[j].first) / (points[i].first - points[j].first);
            }
        }
        secret += term;
    }

    return secret;
}

int main() {
    try {
        // Read JSON input
        JSON input;
        input.parse(std::cin);

        if (input.data.find("keys") == input.data.end()) {
            throw std::runtime_error("Missing 'keys' in input");
        }

        int n = std::stoi(input.data["keys"]["n"]);
        int k = std::stoi(input.data["keys"]["k"]);

        std::vector<std::pair<int64_t, int64_t>> points;

        // Process each point
        for (const auto& item : input.data) {
            if (item.first != "keys") {
                int64_t x = std::stoll(item.first);
                if (item.second.find("base") == item.second.end() || item.second.find("value") == item.second.end()) {
                    throw std::runtime_error("Missing 'base' or 'value' for point " + item.first);
                }
                int base = std::stoi(item.second.at("base"));
                int64_t y = convert_to_decimal(item.second.at("value"), base);
                points.emplace_back(x, y);
            }
        }

        // Ensure we have at least k points
        if (points.size() < k) {
            throw std::runtime_error("Not enough points provided. Need at least " + std::to_string(k) + " points, but got " + std::to_string(points.size()));
        }

        // Use the first k points to find the secret
        std::vector<std::pair<int64_t, int64_t>> used_points(points.begin(), points.begin() + k);
        int64_t secret = find_secret(used_points);

        // Output the result
        std::cout << secret << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

//Test Case 1
/*{
    "keys": {
        "n": 4,
        "k": 3
    },
    "1": {
        "base": "10",
        "value": "4"
    },
    "2": {
        "base": "2",
        "value": "111"
    },
    "3": {
        "base": "10",
        "value": "12"
    },
    "6": {
        "base": "4",
        "value": "213"
    }
}

Output: 3

*/

// Test Case 2
/*
{
    "keys": {
        "n": 9,
        "k": 6
    },
    "1": {
        "base": "10",
        "value": "28735619723837"
    },
    "2": {
        "base": "16",
        "value": "1A228867F0CA"
    },
    "3": {
        "base": "12",
        "value": "32811A4AA0B7B"
    },
    "4": {
        "base": "11",
        "value": "917978721331A"
    },
    "5": {
        "base": "16",
        "value": "1A22886782E1"
    },
    "6": {
        "base": "10",
        "value": "28735619654702"
    },
    "7": {
        "base": "14",
        "value": "71AB5070CC4B"
    },
    "8": {
        "base": "9",
        "value": "122662581541670"
    },
    "9": {
        "base": "8",
        "value": "642121030037605"
    }
}

Output: 28735619723846

*/