#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <stddef.h>

//-----------------------------------------------------------------------------
// Gene‐map data structures
//-----------------------------------------------------------------------------

struct GeneModel {
    std::string symbol;
    std::string chromosome;
    int         start  = 0;
    int         end    = 0;
    double      expressionLevel = 0.0;
    double      polygenicScore  = 0.0;
    bool        isKnockout      = false;
    std::vector<std::string> disorderTags;
    std::map<std::string, double> brainRegionExpression;
};

struct GenomeStats {
    int    totalGenes     = 0;
    int    totalKnockouts = 0;
    double avgExpression  = 0.0;
    double avgPolyScore   = 0.0;
    std::string timestamp;
};

//-----------------------------------------------------------------------------
// Pathway data structures
//-----------------------------------------------------------------------------

struct Pathway {
    std::string name;
    std::string description;
    std::vector<std::string> geneSymbols;
    std::map<std::string, std::vector<std::string>> interactions;
};

//-----------------------------------------------------------------------------
// Gene set data structures
//-----------------------------------------------------------------------------

struct GeneSet {
    std::string name;
    std::vector<std::string> geneSymbols;
};

class AlignmentMap {
public:
    void addGene(const GeneModel& g);
    const std::vector<GeneModel>& getGenes() const;
    GenomeStats calculateStatistics() const;
    void loadGenesFromCSV(const std::string& filename);
    void loadGenesFromJSON(const std::string& filename);
    void toggleKnockout(const std::string& symbol);
    void addPathway(const Pathway& p);
    const std::vector<Pathway>& getPathways() const;
    void addGeneSet(const GeneSet& gs);
    const std::vector<GeneSet>& getGeneSets() const;

private:
    std::vector<GeneModel> genes_;
    std::vector<Pathway> pathways_;
    std::vector<GeneSet> geneSets_;
    std::string makeTimestamp() const;
};

AlignmentMap createDemoMap();
std::vector<Pathway> createDemoPathways();

//-----------------------------------------------------------------------------
// Sequence‐alignment data structures
//-----------------------------------------------------------------------------

enum class SequenceType { DNA, RNA, Protein };

struct SequenceModel {
    std::string   name;
    SequenceType  type;
    std::string   raw;     // original sequence
    std::string   aligned; // current aligned sequence
};

struct AlignmentBlock {
    std::string              reference;
    std::vector<SequenceModel> sequences;
    int cursorPos   = 0;
    int selectedSeq = 0;
};

class AlignmentEditor {
public:
    void loadDemoDNA();
    void render(int width, int height) const;
    void loadSequencesFromCSV(const std::string& filename);
    void loadSequencesFromJSON(const std::string& filename);
    // navigation / editing
    void moveCursor(int delta);
    void selectSequence(int delta);
    void toggleGap();
    void reverseComplementSelected();
    void editSelectedBase(char base);

    // Public for testing purposes
    const std::vector<SequenceModel>& getSequences() const;

private:
    AlignmentBlock block_;

    char complement(char base, SequenceType type) const;
};
