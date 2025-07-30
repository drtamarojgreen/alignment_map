#include "api_logic.h"
#include "test_runner.h"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// Test that the fetch function throws an exception when the HTTP getter returns an empty string.
TEST_CASE(ApiLogic_FetchDataThrowsOnEmptyResponse) {
    // Given a list of gene accessions
    std::vector<std::string> accessions = {"BRCA1"};

    // When fetching data with a getter that returns an empty response
    auto mock_getter = [](const std::string& url, const std::string& api_key) -> std::string {
        return ""; // Simulate a failed HTTP request
    };

    // Then it should throw a runtime_error.
    bool thrown = false;
    try {
        fetchGeneDataFromNCBI(accessions, "", mock_getter);
    } catch (const std::runtime_error& e) {
        thrown = true;
        ASSERT_EQUAL(std::string(e.what()), std::string("Failed to get a response from NCBI API."));
    }
    ASSERT_TRUE(thrown);
}

// Test parsing of a complex JSON file with multiple genes, nested objects, and arrays.
TEST_CASE(ApiLogic_ParsingComplexJson) {
    // Given a mock HTTP getter that returns the content of our test JSON file
    auto mock_getter = [](const std::string& url, const std::string& api_key) -> std::string {
        std::ifstream file("tests/new_gene_data.json");
        if (!file.is_open()) {
            return "{}"; // Return empty object if file not found
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    };

    // When fetching data
    std::vector<std::string> accessions = {"BRCA1", "HTT", "APOE"};
    std::vector<GeneModel> genes = fetchGeneDataFromNCBI(accessions, "", mock_getter);

    // Then the returned data should be parsed correctly
    ASSERT_EQUAL(genes.size(), 3);

    // --- Verify Gene 1: BRCA1 ---
    const auto& gene1 = genes[0];
    ASSERT_EQUAL(gene1.symbol, "BRCA1");
    ASSERT_EQUAL(gene1.isKnockout, false);
    ASSERT_EQUAL(gene1.expressionLevel, 9.2);
    // Check disorder tags
    ASSERT_EQUAL(gene1.disorderTags.size(), 2);
    ASSERT_EQUAL(gene1.disorderTags[0], "Breast Cancer");
    ASSERT_EQUAL(gene1.disorderTags[1], "Ovarian Cancer");
    // Check brain region expression
    ASSERT_EQUAL(gene1.brainRegionExpression.size(), 2);
    ASSERT_EQUAL(gene1.brainRegionExpression.at("Frontal Lobe"), 0.4);
    ASSERT_EQUAL(gene1.brainRegionExpression.at("Temporal Lobe"), 0.5);

    // --- Verify Gene 2: HTT ---
    const auto& gene2 = genes[1];
    ASSERT_EQUAL(gene2.symbol, "HTT");
    ASSERT_EQUAL(gene2.isKnockout, true);
    ASSERT_EQUAL(gene2.expressionLevel, 1.8);
    // Check disorder tags
    ASSERT_EQUAL(gene2.disorderTags.size(), 1);
    ASSERT_EQUAL(gene2.disorderTags[0], "Huntington's Disease");
    // Check brain region expression (should be empty)
    ASSERT_EQUAL(gene2.brainRegionExpression.size(), 0);

    // --- Verify Gene 3: APOE ---
    const auto& gene3 = genes[2];
    ASSERT_EQUAL(gene3.symbol, "APOE");
    ASSERT_EQUAL(gene3.isKnockout, false);
    ASSERT_EQUAL(gene3.expressionLevel, 7.5);
    // Check disorder tags (should be empty)
    ASSERT_EQUAL(gene3.disorderTags.size(), 0);
    // Check brain region expression
    ASSERT_EQUAL(gene3.brainRegionExpression.size(), 1);
    ASSERT_EQUAL(gene3.brainRegionExpression.at("Parietal Lobe"), 0.9);
}
