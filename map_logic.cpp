
#include "map_logic.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <fstream>
#include <sstream>
#include <vector>
#include <cstddef>

//-----------------------------------------------------------------------------
// AlignmentMap additional methods
//-----------------------------------------------------------------------------
void AlignmentMap::loadGenesFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open CSV file " << filename << std::endl;
        return;
    }

    std::string line;
    // Skip header
    if (!std::getline(file, line)) return;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> fields;
        while (std::getline(ss, token, ',')) {
            fields.push_back(token);
        }

        // Require at least 4 fields (gene_name, knockout, status, expression_level)
        if (fields.size() < 4) {
            std::cerr << "Warning: Skipping malformed CSV line: " << line << std::endl;
            continue;
        }

        GeneModel g;
        g.symbol = fields[0];
        g.isKnockout = (fields[1] == "X");  // 'X' indicates knockout
        // Ignore status field or map if needed
        try {
            g.expressionLevel = std::stod(fields[3]);
        } catch (const std::exception&) {
            std::cerr << "Warning: Invalid expression level in line: " << line << std::endl;
            continue;
        }

        if (fields.size() > 4) {
            std::stringstream ss(fields[4]);
            std::string tag;
            while(std::getline(ss, tag, ';')) {
                g.disorderTags.push_back(tag);
            }
        }

        if (fields.size() > 5) {
            std::stringstream ss(fields[5]);
            std::string kv;
            while(std::getline(ss, kv, ';')) {
                size_t sep = kv.find(':');
                if (sep != std::string::npos) {
                    std::string region = kv.substr(0, sep);
                    double expr = std::stod(kv.substr(sep + 1));
                    g.brainRegionExpression[region] = expr;
                }
            }
        }

        // Set defaults for missing fields
        g.chromosome = "unknown";
        g.start = 0;
        g.end = 0;
        g.polygenicScore = 0.0;

        addGene(g);
    }
}

void AlignmentMap::loadGenesFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open JSON file " << filename << std::endl;
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Basic JSON parsing - find "genes" array and parse objects
    // Note: For production, use a proper JSON library like nlohmann/json
    size_t pos = content.find("\"genes\": [");
    if (pos == std::string::npos) {
        std::cerr << "Error: No 'genes' array found in JSON" << std::endl;
        return;
    }

    // Simplified parsing - extract objects (this is basic and may need enhancement)
    size_t start = content.find('[', pos);
    size_t end = content.find(']', start);
    if (start == std::string::npos || end == std::string::npos) return;

    std::string arrayContent = content.substr(start + 1, end - start - 1);

    // Split by objects (simplified)
    size_t objStart = 0;
    while ((objStart = arrayContent.find('{', objStart)) != std::string::npos) {
        size_t objEnd = arrayContent.find('}', objStart);
        if (objEnd == std::string::npos) break;

        std::string obj = arrayContent.substr(objStart, objEnd - objStart + 1);

        // Extract fields (basic string search)
        GeneModel g;

        // gene_name
        size_t namePos = obj.find("\"gene_name\":");
        if (namePos != std::string::npos) {
            size_t quote1 = obj.find('"', namePos + 13);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                g.symbol = obj.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        // knockout
        size_t koPos = obj.find("\"knockout\":");
        if (koPos != std::string::npos) {
            size_t truePos = obj.find("true", koPos);
            g.isKnockout = (truePos != std::string::npos && truePos < obj.find(',', koPos));
        }

        // expression_level
        size_t exprPos = obj.find("\"expression_level\":");
        if (exprPos != std::string::npos) {
            size_t numStart = obj.find_first_of("0123456789.", exprPos + 20);
            size_t numEnd = obj.find_first_not_of("0123456789.", numStart);
            if (numStart != std::string::npos) {
                std::string numStr = obj.substr(numStart, numEnd - numStart);
                try {
                    g.expressionLevel = std::stod(numStr);
                } catch (...) {}
            }
        }

        // disorderTags
        size_t tagsPos = obj.find("\"disorderTags\":");
        if (tagsPos != std::string::npos) {
            size_t start = obj.find('[', tagsPos);
            size_t end = obj.find(']', start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string tagsStr = obj.substr(start + 1, end - start - 1);
                std::stringstream ss(tagsStr);
                std::string tag;
                while(std::getline(ss, tag, ',')) {
                    size_t quote1 = tag.find('"');
                    size_t quote2 = tag.find('"', quote1 + 1);
                    if (quote1 != std::string::npos && quote2 != std::string::npos) {
                        g.disorderTags.push_back(tag.substr(quote1 + 1, quote2 - quote1 - 1));
                    }
                }
            }
        }

        // brainRegionExpression
        size_t brainPos = obj.find("\"brainRegionExpression\":");
        if (brainPos != std::string::npos) {
            size_t start = obj.find('{', brainPos);
            size_t end = obj.find('}', start);
            if (start != std::string::npos && end != std::string::npos) {
                std::string brainStr = obj.substr(start + 1, end - start - 1);
                std::stringstream ss(brainStr);
                std::string kv;
                while(std::getline(ss, kv, ',')) {
                    size_t sep = kv.find(':');
                    if (sep != std::string::npos) {
                        size_t quote1 = kv.find('"');
                        size_t quote2 = kv.find('"', quote1 + 1);
                        if (quote1 != std::string::npos && quote2 != std::string::npos) {
                            std::string region = kv.substr(quote1 + 1, quote2 - quote1 - 1);
                            double expr = std::stod(kv.substr(sep + 1));
                            g.brainRegionExpression[region] = expr;
                        }
                    }
                }
            }
        }

        // Defaults
        g.chromosome = "unknown";
        g.start = 0;
        g.end = 0;
        g.polygenicScore = 0.0;

        addGene(g);
        objStart = objEnd + 1;
    }
}

//-----------------------------------------------------------------------------
// AlignmentEditor additional methods
//-----------------------------------------------------------------------------
void AlignmentEditor::loadSequencesFromCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open CSV file " << filename << std::endl;
        return;
    }

    std::string line;
    // Skip header
    if (!std::getline(file, line)) return;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> fields;
        while (std::getline(ss, token, ',')) {
            fields.push_back(token);
        }

        if (fields.size() < 3) {
            std::cerr << "Warning: Skipping malformed CSV line: " << line << std::endl;
            continue;
        }

        SequenceModel seq;
        seq.name = fields[0];  // sequence_id
        seq.type = SequenceType::DNA;
        seq.raw = fields[1];   // sequence
        seq.aligned = fields[1];  // same as raw initially

        // annotations ignored for now (no field in SequenceModel)

        block_.sequences.push_back(seq);
    }

}

void AlignmentEditor::loadSequencesFromJSON(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open JSON file " << filename << std::endl;
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Similar to gene JSON parsing - find "sequences" array
    size_t pos = content.find("\"sequences\": [");
    if (pos == std::string::npos) {
        std::cerr << "Error: No 'sequences' array found in JSON" << std::endl;
        return;
    }

    // Extract array content (simplified)
    size_t start = content.find('[', pos);
    size_t end = content.find(']', start);
    if (start == std::string::npos || end == std::string::npos) return;

    std::string arrayContent = content.substr(start + 1, end - start - 1);

    // Parse each object
    size_t objStart = 0;
    while ((objStart = arrayContent.find('{', objStart)) != std::string::npos) {
        size_t objEnd = arrayContent.find('}', objStart);
        if (objEnd == std::string::npos) break;

        std::string obj = arrayContent.substr(objStart, objEnd - objStart + 1);

        SequenceModel seq;
        seq.type = SequenceType::DNA;

        // sequence_id
        size_t idPos = obj.find("\"sequence_id\":");
        if (idPos != std::string::npos) {
            size_t quote1 = obj.find('"', idPos + 15);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                seq.name = obj.substr(quote1 + 1, quote2 - quote1 - 1);
            }
        }

        // sequence
        size_t seqPos = obj.find("\"sequence\":");
        if (seqPos != std::string::npos) {
            size_t quote1 = obj.find('"', seqPos + 12);
            size_t quote2 = obj.find('"', quote1 + 1);
            if (quote1 != std::string::npos && quote2 != std::string::npos) {
                seq.raw = obj.substr(quote1 + 1, quote2 - quote1 - 1);
                seq.aligned = seq.raw;
            }
        }

        // annotations ignored

        block_.sequences.push_back(seq);
        objStart = objEnd + 1;
    }
}

//-----------------------------------------------------------------------------
// AlignmentMap implementation
//-----------------------------------------------------------------------------

void AlignmentMap::addGene(const GeneModel& g) {
    genes_.push_back(g);
}

void AlignmentMap::addPathway(const Pathway& p) {
    pathways_.push_back(p);
}

const std::vector<GeneModel>& AlignmentMap::getGenes() const {
    return genes_;
}

const std::vector<Pathway>& AlignmentMap::getPathways() const {
    return pathways_;
}

void AlignmentMap::addGeneSet(const GeneSet& gs) {
    geneSets_.push_back(gs);
}

const std::vector<GeneSet>& AlignmentMap::getGeneSets() const {
    return geneSets_;
}

GenomeStats AlignmentMap::calculateStatistics() const {
    GenomeStats s;
    s.totalGenes     = int(genes_.size());
    double sumE = 0, sumP = 0;
    for (auto& g : genes_) {
        sumE += g.expressionLevel;
        sumP += g.polygenicScore;
        if (g.isKnockout) s.totalKnockouts++;
    }
    if (s.totalGenes > 0) {
        s.avgExpression = sumE / s.totalGenes;
        s.avgPolyScore  = sumP / s.totalGenes;
    }
    s.timestamp = makeTimestamp();
    return s;
}

void AlignmentMap::toggleKnockout(const std::string& symbol) {
    for (auto& g : genes_) {
        if (g.symbol == symbol) {
            g.isKnockout = !g.isKnockout;
            return;
        }
    }
}

std::string AlignmentMap::makeTimestamp() const {
    auto now = std::time(nullptr);
    std::tm tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &now);
#else
    tm = *std::localtime(&now);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

AlignmentMap createDemoMap() {
    AlignmentMap m;
    m.addGene({"COMT","22",19929000,19957000,7.5,0.85,false});
    m.addGene({"DRD2","11",113409000,113475000,6.2,0.72,false});
    m.addGene({"BDNF","11",27650000,27700000,8.1,0.60,false});
    return m;
}

std::vector<Pathway> createDemoPathways() {
    std::vector<Pathway> pathways;

    Pathway p1;
    p1.name = "Neural Plasticity";
    p1.description = "Pathway involved in learning and memory";
    p1.geneSymbols = {"BDNF", "CREB1", "GRIN2B", "CAMK2A"};
    p1.interactions = {
        {"BDNF", {"CREB1", "CAMK2A"}},
        {"CREB1", {"GRIN2B"}},
        {"CAMK2A", {"GRIN2B"}}
    };
    pathways.push_back(p1);

    Pathway p2;
    p2.name = "Caspase-Mediated Apoptosis";
    p2.description = "Pathway involved in programmed cell death";
    p2.geneSymbols = {"CASP3", "CASP8", "CASP9", "BCL2"};
    p2.interactions = {
        {"CASP8", {"CASP3"}},
        {"CASP9", {"CASP3"}},
        {"BCL2", {"CASP9"}}
    };
    pathways.push_back(p2);

    return pathways;
}

//-----------------------------------------------------------------------------
// AlignmentEditor implementation
//-----------------------------------------------------------------------------

void AlignmentEditor::loadDemoDNA() {
    block_.reference =
      "ATCGATCGATCGATCG";
    block_.sequences = {
        {"GeneA", SequenceType::DNA,
          "ATCGATCGATCGATCG","ATCGATCGATCGATCG"},
        {"GeneB", SequenceType::DNA,
          "AT-GATTGATCGATCG","AT-GATTGATCGATCG"},
        {"GeneC", SequenceType::DNA,
          "ATCG-TCGAT-GATCG","ATCG-TCGAT-GATCG"}
    };
}

void AlignmentEditor::render(int width, int height) const {
#if defined(_WIN32) || defined(_WIN64)
    // Header
    COORD pos{0,0};
    auto hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOut, pos);
    std::cout
      << "MSA Editor [Esc=Back]";

    // Reference
    pos.Y = 1;
    SetConsoleCursorPosition(hOut, pos);
    std::cout << "Ref: " << block_.reference;

    // Sequences
    int lines = height - 3;
    for (int i = 0; i < lines && i < int(block_.sequences.size()); ++i) {
        pos.Y = SHORT(2 + i);
        SetConsoleCursorPosition(hOut, pos);

        auto& s = block_.sequences[i];
        std::cout << (i==block_.selectedSeq ? "> " : "  ")
                  << s.name << ": ";

        for (int j = 0; j < int(s.aligned.size()) && j < width - 10; ++j) {
            bool sel = (i==block_.selectedSeq && j==block_.cursorPos);
            std::cout << (sel ? '[' : ' ')
                      << s.aligned[j]
                      << (sel ? ']' : ' ');
        }
    }

    // Footer
    pos.Y = SHORT(height-1);
    SetConsoleCursorPosition(hOut, pos);
    std::cout
      << "[<-->]Move Cursor  [^/v]Select Seq  "
      << "[L]Load  [G]Gap  [R]RevComp  [E]Edit";
#else
    // Non-Windows stub
    (void)width; // unused
    (void)height; // unused
    std::cout << "Rendering is not supported on this platform." << std::endl;
#endif
}

void AlignmentEditor::moveCursor(int delta) {
    int len = int(block_.reference.size());
    block_.cursorPos =
      std::clamp(block_.cursorPos + delta, 0, len-1);
}

void AlignmentEditor::selectSequence(int delta) {
    int n = int(block_.sequences.size());
    block_.selectedSeq =
      (block_.selectedSeq + delta + n) % n;
}

void AlignmentEditor::toggleGap() {
    auto& s = block_.sequences[block_.selectedSeq];
    int p = block_.cursorPos;
    if (p<0||p>=int(s.aligned.size())) return;
    s.aligned[p] = (s.aligned[p]=='-' ? s.raw[p] : '-');
}

void AlignmentEditor::reverseComplementSelected() {
    auto& s = block_.sequences[block_.selectedSeq];
    std::string rev;
    for (auto it = s.aligned.rbegin(); it != s.aligned.rend(); ++it)
        rev.push_back(complement(*it, s.type));
    s.aligned = std::move(rev);
}

void AlignmentEditor::editSelectedBase(char base) {
    auto& s = block_.sequences[block_.selectedSeq];
    int p = block_.cursorPos;
    if (p<0||p>=int(s.aligned.size())) return;
    s.aligned[p] = std::toupper(base);
}

const std::vector<SequenceModel>& AlignmentEditor::getSequences() const {
    return block_.sequences;
}

char AlignmentEditor::complement(char b, SequenceType t) const {
    switch (std::toupper(b)) {
        case 'A': return (t==SequenceType::RNA)? 'U':'T';
        case 'T': return 'A';
        case 'U': return 'A';
        case 'C': return 'G';
        case 'G': return 'C';
        default:  return b;
    }
}
