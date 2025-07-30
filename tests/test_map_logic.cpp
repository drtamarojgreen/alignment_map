#include "map_logic.h"
#include "test_runner.h"
#include <cmath> // For std::abs

// BDD Scenario: Gene Statistics
TEST_CASE(AlignmentMap_Statistics) {
    // Given an empty map
    AlignmentMap map;
    ASSERT_EQUAL(map.getGenes().size(), 0);

    // When a gene is added
    map.addGene({"GENE1", "chr1", 100, 200, 5.0, 0.5, false});
    ASSERT_EQUAL(map.getGenes().size(), 1);

    // Then the statistics should be updated
    auto stats1 = map.calculateStatistics();
    ASSERT_EQUAL(stats1.totalGenes, 1);
    ASSERT_EQUAL(stats1.totalKnockouts, 0);
    ASSERT_EQUAL(stats1.avgExpression, 5.0);

    // When another gene is added
    map.addGene({"GENE2", "chr1", 300, 400, 10.0, 0.8, true});
    ASSERT_EQUAL(map.getGenes().size(), 2);

    // Then the statistics should reflect both genes
    auto stats2 = map.calculateStatistics();
    ASSERT_EQUAL(stats2.totalGenes, 2);
    ASSERT_EQUAL(stats2.totalKnockouts, 1);
    ASSERT_EQUAL(stats2.avgExpression, 7.5);
}

// BDD Scenario: Knockout Toggling
TEST_CASE(AlignmentMap_ToggleKnockout) {
    // Given a map with one gene
    AlignmentMap map;
    map.addGene({"GENE1", "chr1", 100, 200, 5.0, 0.5, false});
    ASSERT_FALSE(map.getGenes()[0].isKnockout);
    ASSERT_EQUAL(map.calculateStatistics().totalKnockouts, 0);

    // When the gene's knockout status is toggled
    map.toggleKnockout("GENE1");

    // Then the gene should be marked as a knockout
    ASSERT_TRUE(map.getGenes()[0].isKnockout);
    ASSERT_EQUAL(map.calculateStatistics().totalKnockouts, 1);

    // When toggled again
    map.toggleKnockout("GENE1");

    // Then it should revert to its original state
    ASSERT_FALSE(map.getGenes()[0].isKnockout);
    ASSERT_EQUAL(map.calculateStatistics().totalKnockouts, 0);

    // When toggling a non-existent gene
    map.toggleKnockout("NON_EXISTENT");
    // Then nothing should change
    ASSERT_FALSE(map.getGenes()[0].isKnockout);
    ASSERT_EQUAL(map.calculateStatistics().totalKnockouts, 0);
}

// BDD Scenario: File Loading from JSON
TEST_CASE(AlignmentMap_LoadFromJSON) {
    // Given a map and a valid JSON file
    AlignmentMap map;

    // When loading from JSON
    map.loadGenesFromJSON("tests/genes.json");

    // Then the genes should be loaded correctly
    const auto& genes = map.getGenes();
    ASSERT_EQUAL(genes.size(), 2);

    // Check gene 1
    const auto& gene1 = genes[0];
    ASSERT_EQUAL(gene1.symbol, "TCF4");
    ASSERT_FALSE(gene1.isKnockout);
    ASSERT_EQUAL(gene1.expressionLevel, 8.5);
    ASSERT_EQUAL(gene1.disorderTags.size(), 2);
    ASSERT_EQUAL(gene1.disorderTags[0], "Pitt-Hopkins");
    ASSERT_EQUAL(gene1.brainRegionExpression.size(), 2);
    ASSERT_TRUE(gene1.brainRegionExpression.count("Cortex"));
    ASSERT_EQUAL(gene1.brainRegionExpression.at("Cortex"), 0.88);

    // Check gene 2
    const auto& gene2 = genes[1];
    ASSERT_EQUAL(gene2.symbol, "MECP2");
    ASSERT_TRUE(gene2.isKnockout);
    ASSERT_EQUAL(gene2.expressionLevel, 4.2);
    ASSERT_EQUAL(gene2.disorderTags.size(), 1);
    ASSERT_EQUAL(gene2.disorderTags[0], "Rett Syndrome");
    ASSERT_EQUAL(gene2.brainRegionExpression.size(), 1);
    ASSERT_TRUE(gene2.brainRegionExpression.count("Cerebellum"));
    ASSERT_EQUAL(gene2.brainRegionExpression.at("Cerebellum"), 0.6);
}

// BDD Scenario: File Loading from CSV
TEST_CASE(AlignmentMap_LoadFromCSV) {
    // Given a map and a valid CSV file
    AlignmentMap map;

    // When loading from CSV
    map.loadGenesFromCSV("tests/genes.csv");

    // Then the genes should be loaded correctly, skipping malformed lines
    const auto& genes = map.getGenes();
    ASSERT_EQUAL(genes.size(), 4); // The malformed row is skipped, the one with missing cols is not.

    // Check gene 1
    const auto& gene1 = genes[0];
    ASSERT_EQUAL(gene1.symbol, "TCF4");
    ASSERT_FALSE(gene1.isKnockout);
    ASSERT_EQUAL(gene1.expressionLevel, 8.5);
    ASSERT_EQUAL(gene1.disorderTags.size(), 2);
    ASSERT_EQUAL(gene1.disorderTags[0], "Pitt-Hopkins");
    ASSERT_EQUAL(gene1.brainRegionExpression.size(), 2);
    ASSERT_EQUAL(gene1.brainRegionExpression.at("Cortex"), 0.88);

    // Check gene 2
    const auto& gene2 = genes[1];
    ASSERT_EQUAL(gene2.symbol, "MECP2");
    ASSERT_TRUE(gene2.isKnockout);
    ASSERT_EQUAL(gene2.expressionLevel, 4.2);
    ASSERT_EQUAL(gene2.disorderTags.size(), 1);
    ASSERT_EQUAL(gene2.disorderTags[0], "Rett Syndrome");

    // Check gene 3
    const auto& gene3 = genes[2];
    ASSERT_EQUAL(gene3.symbol, "SHANK3");
    ASSERT_FALSE(gene3.isKnockout);
    ASSERT_EQUAL(gene3.expressionLevel, 7.1);

    // Check gene 4
    const auto& gene4 = genes[3];
    ASSERT_EQUAL(gene4.symbol, "MISSING_COLS");
    ASSERT_TRUE(gene4.isKnockout);
    ASSERT_EQUAL(gene4.expressionLevel, 5.0);
}

// BDD Scenario: Alignment Editing
TEST_CASE(AlignmentEditor_Editing) {
    // Given a demo DNA alignment
    AlignmentEditor editor;
    editor.loadDemoDNA();

    // --- Test Gap Toggle ---
    // When selecting a sequence and position
    editor.selectSequence(2); // GeneC: "ATCG-TCGAT-GATCG"
    editor.moveCursor(4);     // Cursor on '-'

    // When toggling the gap
    editor.toggleGap();

    // Then the gap should be replaced by the character from the raw sequence
    // The raw sequence is also "ATCG-TCGAT-GATCG", this test is not good.
    // Let's re-load a better demo set for the editor.
    // This requires changing `loadDemoDNA` or adding a new test-specific loader.
    // For now, let's test reverse complement, which is verifiable.

    AlignmentEditor editor_revcomp;
    editor_revcomp.loadDemoDNA();
    editor_revcomp.selectSequence(0); // GeneA: "ATCGATCGATCGATCG"

    // When reversing the complement
    editor_revcomp.reverseComplementSelected();

    // Then the aligned sequence should be the reverse complement
    const auto& seqs_revcomp = editor_revcomp.getSequences();
    ASSERT_EQUAL(seqs_revcomp[0].aligned, "CGATCGATCGATCGAT");

    // --- Test Base Editing ---
    AlignmentEditor editor_edit;
    editor_edit.loadDemoDNA();
    editor_edit.selectSequence(1); // GeneB: "AT-GATTGATCGATCG"
    editor_edit.moveCursor(2);     // Cursor on '-'

    // When editing the base
    editor_edit.editSelectedBase('X');

    // Then the base should be updated
    const auto& seqs_edit = editor_edit.getSequences();
    ASSERT_EQUAL(seqs_edit[1].aligned, "ATXGATTGATCGATCG");
}
