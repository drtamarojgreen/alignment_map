#include "api_logic.h"
#include <stdexcept> // For std::runtime_error
#include <sstream>   // For std::stringstream
#include <vector>
#include <map>
#include <algorithm>
#include <functional>

// --- Private Helper Function Declarations ---

// Performs a basic HTTP GET request.
// NOTE: This is a placeholder for a platform-specific implementation (e.g., using WinINet on Windows).
static std::string httpGetRequest(const std::string& url, const std::string& api_key);

// Parses a JSON response from the NCBI Gene API.
// NOTE: This is a placeholder for a minimal, purpose-built parser. It is NOT a full JSON parser.
static std::vector<GeneModel> parseGeneJson(const std::string& json_response);

// --- Public Function Implementations ---

std::vector<GeneModel> fetchGeneDataFromNCBI(
    const std::vector<std::string>& gene_accessions,
    const std::string& api_key,
    const std::function<std::string(const std::string&, const std::string&)>& http_getter) {
    if (gene_accessions.empty()) {
        return {};
    }

    // 1. Construct the URL for the NCBI API request.
    const std::string base_url = "https://api.ncbi.nlm.nih.gov/datasets/v2alpha/gene/accession/";
    std::stringstream url_stream;
    url_stream << base_url;
    for (size_t i = 0; i < gene_accessions.size(); ++i) {
        url_stream << gene_accessions[i] << (i == gene_accessions.size() - 1 ? "" : ",");
    }
    // Add parameters to get gene report
    url_stream << "?table_fields=gene-id&table_fields=symbol&table_fields=description&table_fields=genomic-ranges";

    // 2. Perform the HTTP GET request.
    std::string json_response;
    if (http_getter) {
        json_response = http_getter(url_stream.str(), api_key);
    } else {
        json_response = httpGetRequest(url_stream.str(), api_key);
    }

    if (json_response.empty()) {
        // Handle case where the request failed at the HTTP level
        throw std::runtime_error("Failed to get a response from NCBI API.");
    }

    // 3. Parse the JSON response into GeneModel objects.
    return parseGeneJson(json_response);
}

// --- Private Helper Function Implementations ---

// Performs a basic HTTP GET request.
// NOTE: This is a placeholder for a platform-specific implementation (e.g., using WinINet on Windows).
static std::string httpGetRequest(const std::string& url, const std::string& api_key) {
    // This is a dummy implementation for compilation and testing purposes.
    // It does not perform a real HTTP request.
    return "";
}

// --- Minimalist JSON Parser Helper Functions ---

// Trim whitespace from a string.
static std::string trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return s;
    }
    size_t last = s.find_last_not_of(" \t\n\r");
    return s.substr(first, (last - first + 1));
}

// Finds the value part of a key-value pair. A simple parser that assumes values don't contain commas or braces.
static std::string find_value_simple(const std::string& json_blob, const std::string& key) {
    std::string search_key = "\"" + key + "\":";
    size_t key_pos = json_blob.find(search_key);
    if (key_pos == std::string::npos) return "";

    size_t value_start = key_pos + search_key.length();
    size_t value_end = json_blob.find_first_of(",}", value_start);
    return trim(json_blob.substr(value_start, value_end - value_start));
}

// Finds a complete value, handling nested structures like objects or arrays.
static std::string find_value_complex(const std::string& json_blob, const std::string& key) {
    std::string search_key = "\"" + key + "\":";
    size_t key_pos = json_blob.find(search_key);
    if (key_pos == std::string::npos) return "";

    size_t value_start = key_pos + search_key.length();
    value_start = json_blob.find_first_not_of(" \t\n\r", value_start);
    if (value_start == std::string::npos) return "";

    char start_char = json_blob[value_start];
    char end_char = ' ';
    if (start_char == '{') end_char = '}';
    else if (start_char == '[') end_char = ']';
    else { // For simple values (string, bool, number)
        size_t value_end = json_blob.find_first_of(",}", value_start);
        return trim(json_blob.substr(value_start, value_end - value_start));
    }

    int balance = 1;
    size_t value_end = value_start + 1;
    while (value_end < json_blob.length() && balance > 0) {
        if (json_blob[value_end] == start_char) balance++;
        if (json_blob[value_end] == end_char) balance--;
        value_end++;
    }

    return trim(json_blob.substr(value_start, value_end - value_start));
}

static std::string parse_string_value(const std::string& value) {
    if (value.length() < 2 || value.front() != '"' || value.back() != '"') return "";
    return value.substr(1, value.length() - 2);
}

static bool parse_bool_value(const std::string& value) {
    return value == "true";
}

static double parse_double_value(const std::string& value) {
    try {
        return std::stod(value);
    } catch (...) {
        return 0.0;
    }
}

static std::vector<std::string> parse_string_array(std::string value) {
    std::vector<std::string> result;
    if (value.length() < 2 || value.front() != '[' || value.back() != ']') return result;
    value = value.substr(1, value.length() - 2); // Remove brackets

    std::stringstream ss(value);
    std::string item;
    while(std::getline(ss, item, ',')) {
        result.push_back(parse_string_value(trim(item)));
    }
    return result;
}

static std::map<std::string, double> parse_key_double_map(std::string value) {
    std::map<std::string, double> result;
    if (value.length() < 2 || value.front() != '{' || value.back() != '}') return result;
    value = value.substr(1, value.length() - 2); // Remove braces
    value.erase(std::remove(value.begin(), value.end(), '\n'), value.end());

    std::stringstream ss(value);
    std::string pair_str;
    while(std::getline(ss, pair_str, ',')) {
        size_t colon_pos = pair_str.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = parse_string_value(trim(pair_str.substr(0, colon_pos)));
            double val = parse_double_value(trim(pair_str.substr(colon_pos + 1)));
            if (!key.empty()) {
                result[key] = val;
            }
        }
    }
    return result;
}

// Parses a JSON response from the NCBI Gene API.
// NOTE: This is a placeholder for a minimal, purpose-built parser. It is NOT a full JSON parser.
static std::vector<GeneModel> parseGeneJson(const std::string& json_response) {
    std::vector<GeneModel> models;

    size_t genes_array_start = json_response.find("\"genes\": [");
    if (genes_array_start == std::string::npos) {
        return models;
    }
    genes_array_start += 10; // Move past '"genes": ['

    size_t search_pos = genes_array_start;
    while (search_pos < json_response.length()) {
        size_t obj_start = json_response.find('{', search_pos);
        if (obj_start == std::string::npos) break;

        int brace_count = 1;
        size_t obj_end = obj_start + 1;
        while (obj_end < json_response.length() && brace_count > 0) {
            if (json_response[obj_end] == '{') brace_count++;
            if (json_response[obj_end] == '}') brace_count--;
            obj_end++;
        }

        if (brace_count == 0) {
            std::string gene_blob = json_response.substr(obj_start, obj_end - obj_start);
            GeneModel model;
            model.symbol = parse_string_value(find_value_simple(gene_blob, "gene_name"));
            model.isKnockout = parse_bool_value(find_value_simple(gene_blob, "knockout"));
            model.expressionLevel = parse_double_value(find_value_simple(gene_blob, "expression_level"));
            model.disorderTags = parse_string_array(find_value_complex(gene_blob, "disorderTags"));
            model.brainRegionExpression = parse_key_double_map(find_value_complex(gene_blob, "brainRegionExpression"));
            models.push_back(model);
            search_pos = obj_end;
        } else {
            break; // Malformed JSON
        }
    }
    return models;
}