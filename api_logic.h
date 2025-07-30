#ifndef API_LOGIC_H
#define API_LOGIC_H

#include <string>
#include <vector>
#include <functional>
#include "map_logic.h" // Assuming GeneModel is defined here

// Fetches gene data from the NCBI Datasets API for a given list of gene accessions.
// This is the primary public-facing function for the API module.
//
// @param gene_accessions A vector of strings, where each string is a gene symbol or accession number.
// @param api_key An optional NCBI API key to increase rate limits.
// @param http_getter An optional function to override the default HTTP GET behavior, for testing purposes.
// @return A vector of GeneModel structs populated with the data fetched from the API.
std::vector<GeneModel> fetchGeneDataFromNCBI(
    const std::vector<std::string>& gene_accessions,
    const std::string& api_key = "",
    const std::function<std::string(const std::string& url, const std::string& api_key)>& http_getter = nullptr);

#endif // API_LOGIC_H