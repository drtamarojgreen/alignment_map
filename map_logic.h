#pragma once

#include <string>
#include <vector>
#include <chrono>

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
    std::vector<std::string> categories;
};

struct GenomeStats {
    int    totalGenes     = 0;
    int    totalKnockouts = 0;
    double avgExpression  = 0.0;
    double avgPolyScore   = 0.0;
    std::string timestamp;
};

class AlignmentMap {
public:
    void addGene(const GeneModel& g);
    const std::vector<GeneModel>& getGenes() const;
    GenomeStats calculateStatistics() const;
    void loadGenesFromCSV(const std::string& filename);
    void loadGenesFromJSON(const std::string& filename);
    void toggleKnockout(const std::string& symbol);

private:
    std::vector<GeneModel> genes_;
    std::string makeTimestamp() const;
};

AlignmentMap createDemoMap();

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

private:
    AlignmentBlock block_;

    char complement(char base, SequenceType type) const;
};
