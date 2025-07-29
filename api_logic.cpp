#include "api_logic.h"
#include <stdexcept> // For std::runtime_error
#include <sstream>   // For std::stringstream

// --- Private Helper Function Declarations ---

// Performs a basic HTTP GET request.
// NOTE: This is a placeholder for a platform-specific implementation (e.g., using WinINet on Windows).
static std::string httpGetRequest(const std::string& url, const std::string& api_key);

// Parses a JSON response from the NCBI Gene API.
// NOTE: This is a placeholder for a minimal, purpose-built parser. It is NOT a full JSON parser.
static std::vector<GeneModel> parseGeneJson(const std::string& json_response);

// --- Public Function Implementations ---

std::vector<GeneModel> fetchGeneDataFromNCBI(const std::vector<std::string>& gene_accessions, const std::string& api_key) {
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
    std::string json_response = httpGetRequest(url_stream.str(), api_key);

    if (json_response.empty()) {
        // Handle case where the request failed at the HTTP level
        throw std::runtime_error("Failed to get a response from NCBI API.");
    }

    // 3. Parse the JSON response into GeneModel objects.
    return parseGeneJson(json_response);
}

// --- Private Helper Function Implementations ---

static std::string httpGetRequest(const std::string