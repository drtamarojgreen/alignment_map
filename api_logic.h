#ifndef API_LOGIC_H
#define API_LOGIC_H

#include <string>
#include <vector>
#include "map_logic.h" // Assuming GeneModel is defined here

// Fetches gene data from the NCBI Datasets API for a given list of gene accessions.
// This is the primary public-facing function for the API module.
//
// @param gene_accessions A vector of strings, where each string is a gene symbol or accession number.
// @param api_key An optional NCBI API key to increase rate limits.
// @return A vector of GeneModel structs populated with the data fetched from the API.
std::vector<GeneModel> fetchGeneDataFromNCBI(const std::vector<std::string>& gene_accessions, const std::string& api_key = "");

#endif // API_LOGIC_H