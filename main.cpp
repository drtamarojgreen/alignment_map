#include "map_logic.h"

#include <windows.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <conio.h>
#include <iomanip>
#include <vector>

// Console global handles
static HANDLE hIn, hOut;

// Screen layout
constexpr int SCREEN_W = 80;
constexpr int SCREEN_H = 24;
constexpr int MAP_H    = SCREEN_H/2;
constexpr int STAT_H   = SCREEN_H - MAP_H;

// Camera and navigation constants
constexpr double ANGLE_STEP = 5.0;
constexpr double ZOOM_FACTOR = 1.1;


// UI state
struct Camera { double angle=45, zoom=1.0; int panX=0, panY=0; };
struct UIState  {
    int geneIdx=0, pathwayIdx=0;
    Camera cam;
    bool inAlign=false, inPathway=false;
    std::string statusMessage;
    int statusMessageCounter = 0; // Frames to show message
};

// Forward prototypes
void initConsole();
void clearScreen();
void drawMap(const AlignmentMap&, UIState&);
void drawStats(const AlignmentMap&, UIState&);
void drawAlignment(AlignmentEditor&, UIState&);
void drawPathway(const AlignmentMap&, UIState&);
void handleMainKey(int vk, AlignmentMap&, UIState&);
void handleAlignKey(int vk, AlignmentEditor&, UIState&);

// Helper to show a message for a few frames
void showStatusMessage(const std::string& msg, UIState& st) {
    st.statusMessage = msg;
    st.statusMessageCounter = 3; // Show for ~1-2 seconds
}

// Prompts user for input on the last line of the console without disrupting the UI
std::string promptUser(const std::string& promptText) {
    COORD p{0, SHORT(SCREEN_H - 1)};
    SetConsoleCursorPosition(hOut, p);
    std::cout << std::string(SCREEN_W, ' '); // Clear the line
    SetConsoleCursorPosition(hOut, p);
    std::cout << promptText;

    std::string line;
    char ch;
    while ((ch = _getch()) != '\r') { // Enter key
        if (ch == 27) { // Escape key cancels
            line.clear();
            break;
        }
        if (ch == '\b') {
            if (!line.empty()) {
                line.pop_back();
                std::cout << "\b \b";
            }
        } else {
            if (line.length() < SCREEN_W - promptText.length() - 2) {
                line += ch;
                std::cout << ch;
            }
        }
    }
    return line;
}


// Entry point
int main() {
    // Demo data
    AlignmentMap map = createDemoMap();
    auto pathways = createDemoPathways();
    for(const auto& p : pathways) {
        map.addPathway(p);
    }
    AlignmentEditor editor;
    editor.loadDemoDNA();

    UIState st;
    initConsole();

    // Main loop
    while (true) {
        // Read one console event
        INPUT_RECORD rec; DWORD cnt;
        ReadConsoleInput(hIn, &rec, 1, &cnt);
        if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
            int vk = rec.Event.KeyEvent.wVirtualKeyCode;
            if (vk == VK_ESCAPE) {
                if (st.inAlign) st.inAlign = false;
                else if (st.inPathway) st.inPathway = false;
                else break;
            }
            else if (vk == 'A' && !st.inAlign && !st.inPathway) {
                st.inAlign = true;
            }
            else if (vk == 'V' && !st.inAlign && !st.inPathway) {
                st.inPathway = true;
            }
            else if (st.inAlign) {
                handleAlignKey(vk, editor, st);
            }
            else {
                handleMainKey(vk, map, st);
            }
        }

        // Redraw
        clearScreen();
        if (st.inAlign) {
            drawAlignment(editor, st);
        } else if (st.inPathway) {
            drawPathway(map, st);
        } else {
            drawMap(map, st);
            drawStats(map, st);
            // Show footer with help text or a status message
            COORD p{0, SHORT(SCREEN_H-1)};
            SetConsoleCursorPosition(hOut, p);
            if (st.statusMessageCounter > 0) {
                std::cout << st.statusMessage << std::string(SCREEN_W - st.statusMessage.length(), ' ');
                st.statusMessageCounter--;
            } else {
                std::string footer = "[A]Align [V]Pathway [L]Load [N/P]Gene [↑↓]Pan [←→]Rot [W/S]Zoom [K]KO [Esc]Quit";
                std::cout << footer << std::string(SCREEN_W - footer.length(), ' ');
            }
        }
    }

    return 0;
}

// initialize console modes
void initConsole() {
    hIn  = GetStdHandle(STD_INPUT_HANDLE);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hIn, &mode);
    SetConsoleMode(hIn,
      ENABLE_WINDOW_INPUT |
      ENABLE_PROCESSED_INPUT
    );
}

// clear the entire screen
void clearScreen() {
    COORD tl{0,0}; DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    FillConsoleOutputCharacter(hOut,' ',
      csbi.dwSize.X*csbi.dwSize.Y, tl, &written);
    SetConsoleCursorPosition(hOut, tl);
}

// draw 3D‐map stub
void drawMap(const AlignmentMap& map, UIState& st) {
    auto& G = map.getGenes();
    for (int y=0; y<MAP_H; ++y) {
        COORD pos{0, SHORT(y)};
        SetConsoleCursorPosition(hOut,pos);
        std::string line(SCREEN_W,' ');
        for (int i=0;i<int(G.size());++i) {
            double mid = (G[i].start + G[i].end)/2.0;
            double x   = std::sin((mid/1e7 + st.cam.panX/10.0)
                         + st.cam.angle*M_PI/180.0)
                         * (SCREEN_W/3)*st.cam.zoom
                         + SCREEN_W/2;
            int xi = int(x);
            if (xi>=0 && xi<SCREEN_W && y==MAP_H/2)
                line[xi] = G[i].isKnockout ? 'X' : '*';
        }
        std::cout<<line;
    }
}

// draw stats & selected gene
void drawStats(const AlignmentMap& map, UIState& st) {
    auto stats = map.calculateStatistics();
    auto& G = map.getGenes();
    if (G.empty()) {
        COORD p{0, SHORT(MAP_H)};
        SetConsoleCursorPosition(hOut, p);
        std::cout << "No genes loaded. Press 'L' to load a file.";
        return;
    }
    const auto& g = G[st.geneIdx];

    for (int i=0;i<STAT_H;++i) {
        COORD p{0, SHORT(MAP_H + i)};

    // Draw stats
    SetConsoleCursorPosition(hOut, {0, SHORT(MAP_H)});
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "--- Stats (Updated: " << stats.timestamp << ") ---";
    SetConsoleCursorPosition(hOut, {0, SHORT(MAP_H + 1)});
    std::cout << "Total Genes: " << stats.totalGenes << " | KOs: " << stats.totalKnockouts
              << " | Avg Expr: " << stats.avgExpression;
    SetConsoleCursorPosition(hOut, {0, SHORT(MAP_H + 2)});
    std::cout << std::string(SCREEN_W, '-');

    SetConsoleCursorPosition(hOut, {0, SHORT(MAP_H + 3)});
    std::cout << "Gene: " << g.symbol << " (" << g.chromosome << ":" << g.start << "-" << g.end << ")";
    SetConsoleCursorPosition(hOut, {0, SHORT(MAP_H + 4)});
    std::cout << "ExprLvl: " << g.expressionLevel << " | PScore: " << g.polygenicScore << " | Knockout: " << (g.isKnockout ? "YES" : "no");
    SetConsoleCursorPosition(hOut, {0, SHORT(MAP_H + 5)});
    std::cout << "Disorder Tags: ";
    for (const auto& tag : g.disorderTags) { std::cout << tag << " "; }
    SetConsoleCursorPosition(hOut, {0, SHORT(MAP_H + 6)});
    std::cout << "Brain Region Expression:";
    int i = 0;
    for (const auto& kv : g.brainRegionExpression) {
        SetConsoleCursorPosition(hOut, {SHORT(i == 0 ? 2 : 25), SHORT(MAP_H + 7)});
        std::cout << kv.first << ": " << kv.second;
        i++;

    }
}

// draw alignment editor
void drawAlignment(AlignmentEditor& editor, UIState& st) {
    editor.render(SCREEN_W, SCREEN_H);
}

// draw pathway view
void drawPathway(const AlignmentMap& map, UIState& st) {
    auto& pathways = map.getPathways();
    if (pathways.empty()) {
        std::cout << "No pathways loaded.";
        return;
    }

    st.pathwayIdx = st.pathwayIdx % pathways.size();
    auto& p = pathways[st.pathwayIdx];
    COORD pos{0,0};
    SetConsoleCursorPosition(hOut, pos);

    std::cout << "Pathway: " << p.name << " (" << p.description << ")";
    pos.Y = 2;
    SetConsoleCursorPosition(hOut, pos);

    // crude layout
    std::map<std::string, COORD> genePositions;
    int y = 5;
    for(const auto& symbol : p.geneSymbols) {
        genePositions[symbol] = {10, SHORT(y)};
        y += 2;
    }

    for(const auto& kv : p.interactions) {
        const auto& from = kv.first;
        for(const auto& to : kv.second) {
            if (genePositions.find(from) == genePositions.end() || genePositions.find(to) == genePositions.end()) {
                continue; // Skip drawing if gene isn't in the list
            }
            COORD p1 = genePositions.at(from);
            COORD p2 = genePositions.at(to);

            // very basic line drawing
            int x1 = p1.X, y1 = p1.Y;
            int x2 = p2.X, y2 = p2.Y;

            while(x1 != x2 || y1 != y2) {
                if (x1 < x2) x1++; else if (x1 > x2) x1--;
                if (y1 < y2) y1++; else if (y1 > y2) y1--;
                SetConsoleCursorPosition(hOut, {SHORT(x1), SHORT(y1)});
                std::cout << ".";
            }
        }
    }

    for(const auto& kv : genePositions) {
        SetConsoleCursorPosition(hOut, kv.second);
        std::cout << "[" << kv.first << "]";
    }
}

void handleMainKey(int vk, AlignmentMap& map, UIState& st) {
    if (st.inPathway) {
        switch(vk) {
            case VK_UP:
            case 'P': // Previous
                if (!map.getPathways().empty())
                    st.pathwayIdx = (st.pathwayIdx + map.getPathways().size() - 1) % map.getPathways().size();
                break;
            case VK_DOWN:
            case 'N': // Next
                if (!map.getPathways().empty())
                    st.pathwayIdx = (st.pathwayIdx + 1) % map.getPathways().size();
                break;
        }
        return;
    }
    switch(vk) {
        case VK_LEFT:   st.cam.angle -= ANGLE_STEP; break;
        case VK_RIGHT:  st.cam.angle += ANGLE_STEP; break;
        case VK_UP:     st.cam.panY  -= 1; break;
        case VK_DOWN:   st.cam.panY  += 1; break;
        case 'Q':       st.cam.angle -= 5; break;
        case 'E':       st.cam.angle += 5; break;
        case 'W':       st.cam.zoom  *= ZOOM_FACTOR; break;
        case 'S':       st.cam.zoom  /= ZOOM_FACTOR; break;
        case 'N': // Next Gene
            if (!map.getGenes().empty())
                st.geneIdx = (st.geneIdx + 1) % map.getGenes().size();
            break;
        case 'P': // Previous Gene
            if (!map.getGenes().empty())
                st.geneIdx = (st.geneIdx + map.getGenes().size() - 1) % map.getGenes().size();
            break;
        case 'K':
            if (!map.getGenes().empty())
                map.toggleKnockout(map.getGenes()[st.geneIdx].symbol);
            break;
        case 'L': 
            std::string filepath = promptUser("Load gene file path (or Esc to cancel): ");
            if (filepath.empty()) {
                showStatusMessage("File loading cancelled.", st);
                break;
            }
            // Basic file type detection
            if (filepath.size() > 5 && filepath.substr(filepath.size() - 5) == ".json") {
                map.loadGenesFromJSON(filepath);
                showStatusMessage("Loaded genes from JSON: " + filepath, st);
            } else if (filepath.size() > 4 && filepath.substr(filepath.size() - 4) == ".csv") {
                map.loadGenesFromCSV(filepath);
                showStatusMessage("Loaded genes from CSV: " + filepath, st);
            } else {
                showStatusMessage("Error: Unknown file type for: " + filepath, st);
            }
            st.geneIdx = 0; // Reset index after loading
            break;
        }
   }


void handleAlignKey(int vk, AlignmentEditor& ed, UIState& st) {
    switch(vk) {
        case VK_LEFT:  ed.moveCursor(-1); break;
        case VK_RIGHT: ed.moveCursor(+1); break;
        case VK_UP:    ed.selectSequence(-1); break;
        case VK_DOWN:  ed.selectSequence(+1); break;
        case 'G':      ed.toggleGap(); break;
        case 'R':      ed.reverseComplementSelected(); break;

        case 'E': {
            std::string base = promptUser("Enter new base (or Esc to cancel): ");
            if (!base.empty()) {
                ed.editSelectedBase(base[0]);
            }
            break;
        }

        case 'L': {
            std::string filepath = promptUser("Load sequence file path (or Esc to cancel): ");
            if (filepath.empty()) {
                showStatusMessage("File loading cancelled.", st);
                break;
            }

            if (filepath.size() > 5 && filepath.substr(filepath.size() - 5) == ".json") {
                ed.loadSequencesFromJSON(filepath);
                showStatusMessage("Loaded sequences from JSON: " + filepath, st);
            } else if (filepath.size() > 4 && filepath.substr(filepath.size() - 4) == ".csv") {
                ed.loadSequencesFromCSV(filepath);
                showStatusMessage("Loaded sequences from CSV: " + filepath, st);
            } else {
                showStatusMessage("Error: Unknown file type for: " + filepath, st);
            }

            break;
        }
    }
}
