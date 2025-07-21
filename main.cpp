#include "map_logic.h"

#include <windows.h>
#include <iostream>
#include <cmath>
#include <conio.h>

// Console global handles
static HANDLE hIn, hOut;

// Screen layout
constexpr int SCREEN_W = 80;
constexpr int SCREEN_H = 24;
constexpr int MAP_H    = SCREEN_H/2;
constexpr int STAT_H   = SCREEN_H - MAP_H;

// UI state
struct Camera { double angle=45, zoom=1.0; int panX=0, panY=0; };
struct UIState  { int geneIdx=0, pathwayIdx=0; Camera cam; bool inAlign=false, inPathway=false; };

// Forward prototypes
void initConsole();
void clearScreen();
void drawMap(const AlignmentMap&, UIState&);
void drawStats(const AlignmentMap&, UIState&);
void drawAlignment(AlignmentEditor&, UIState&);
void drawPathway(const AlignmentMap&, UIState&);
void handleMainKey(int vk, AlignmentMap&, UIState&);
void handleAlignKey(int vk, AlignmentEditor&);

std::string readLineFromConsole() {
    std::string line;
    char ch;
    while ((ch = _getch()) != '\r') { // Enter key
        if (ch == '\b') {
            if (!line.empty()) {
                line.pop_back();
                std::cout << "\b \b";
            }
        } else {
            line += ch;
            std::cout << ch;
        }
    }
    std::cout << '\n';
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
                handleAlignKey(vk, editor);
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
            // show mode hint
            COORD p{0, SHORT(SCREEN_H-1)};
            SetConsoleCursorPosition(hOut, p);
            std::cout << "[A]Align  [V]Pathway  [L]Load  [N/P]Next/Prev Gene  [←/→/↑/↓]Pan/Rotate/Zoom  [Esc]Quit";
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
    const auto& g = G[st.geneIdx];

    int y = MAP_H;
    COORD p{0, SHORT(y)};
    SetConsoleCursorPosition(hOut,p);

    std::cout << "Stats:" << std::endl;
    std::cout << " Total:"   << stats.totalGenes << std::endl;
    std::cout << " KO:"      << stats.totalKnockouts << std::endl;
    std::cout << " Expr:"    << stats.avgExpression << std::endl;
    std::cout << " Poly:"    << stats.avgPolyScore << std::endl;
    std::cout << " Updated:" << stats.timestamp << std::endl;
    std::cout << std::endl;
    std::cout << " Gene:"   << g.symbol << std::endl;
    std::cout << " Chr:"    << g.chromosome << " [" << g.start << "-" << g.end << "]" << std::endl;
    std::cout << " ExpLvl:" << g.expressionLevel << std::endl;
    std::cout << " PScore:" << g.polygenicScore << std::endl;
    std::cout << " KO?:"    << (g.isKnockout ? "YES" : "no") << std::endl;

    std::cout << " Tags:";
    for(const auto& tag : g.disorderTags) {
        std::cout << " " << tag;
    }
    std::cout << std::endl;

    std::cout << " Brain Expression:" << std::endl;
    for(const auto&- kv : g.brainRegionExpression) {
        std::cout << "  " << kv.first << ": " << kv.second << std::endl;
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

    for(const auto&- kv : p.interactions) {
        const auto& from = kv.first;
        for(const auto& to : kv.second) {
            COORD p1 = genePositions[from];
            COORD p2 = genePositions[to];

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

    for(const auto&- kv : genePositions) {
        SetConsoleCursorPosition(hOut, kv.second);
        std::cout << "[" << kv.first << "]";
    }
}

void handleMainKey(int vk, AlignmentMap& map, UIState& st) {
    if (st.inPathway) {
        switch(vk) {
            case VK_UP:   st.pathwayIdx = (st.pathwayIdx + map.getPathways().size() - 1) % map.getPathways().size(); break;
            case VK_DOWN: st.pathwayIdx = (st.pathwayIdx + 1) % map.getPathways().size(); break;
        }
        return;
    }
    switch(vk) {
        case VK_LEFT:   st.cam.panX  -= 1; break;
        case VK_RIGHT:  st.cam.panX  += 1; break;
        case VK_UP:     st.cam.panY  -= 1; break;
        case VK_DOWN:   st.cam.panY  += 1; break;
        case 'Q':       st.cam.angle -= 5; break;
        case 'E':       st.cam.angle += 5; break;
        case 'W':       st.cam.zoom  *= 1.1; break;
        case 'S':       st.cam.zoom  /= 1.1; break;
        case 'N':       st.geneIdx = (st.geneIdx + 1) % map.getGenes().size(); break;
        case 'P':       st.geneIdx = (st.geneIdx + map.getGenes().size() - 1) % map.getGenes().size(); break;
        case 'K':
            map.toggleKnockout(map.getGenes()[st.geneIdx].symbol);
            break;

        case 'T': {
            std::cout << "\nEnter disorder tag: ";
            std::string tag = readLineFromConsole();
            if (!tag.empty()) {
                // This is a simplified implementation. A real application would need to modify the underlying data.
                // map.addTagToGene(map.getGenes()[st.geneIdx].symbol, tag);
            }
            break;
        }

        case 'F': {
            std::cout << "\nEnter gene set name to filter by: ";
            std::string setName = readLineFromConsole();
            if (!setName.empty()) {
                // This is a simplified implementation. A real application would need to filter the gene list.
                // map.filterGenesBySet(setName);
            }
            break;
        }

        case 'L': {
            std::cout << "\nLoad genes from: (1) JSON  (2) CSV\nPress key: ";
            char key = _getch();
            std::cout << key << "\n";

            std::cout << "Enter gene file path: ";
            std::string filepath = readLineFromConsole();
            if (filepath.empty()) {
                std::cout << "No file path provided.\n";
                break;
            }

            bool success = false;
            if (key == '1') {
                map.loadGenesFromJSON(filepath);
            } else if (key == '2') {
                map.loadGenesFromCSV(filepath);
            } else {
                std::cout << "Invalid option.\n";
                break;
            }

            break;
        }
    }
}

void handleAlignKey(int vk, AlignmentEditor& ed) {
    switch(vk) {
        case VK_LEFT:  ed.moveCursor(-1); break;
        case VK_RIGHT: ed.moveCursor(+1); break;
        case VK_UP:    ed.selectSequence(-1); break;
        case VK_DOWN:  ed.selectSequence(+1); break;
        case 'G':      ed.toggleGap(); break;
        case 'R':      ed.reverseComplementSelected(); break;

        case 'E': {
            char b = 'A';
            std::cout << "\nBase (A/C/G/T/U): ";
            std::cin >> b;
            ed.editSelectedBase(b);
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }

        case 'L': {
            std::cout << "\nLoad sequences: (1) JSON  (2) CSV\nPress key: ";
            char key = _getch();
            std::cout << key << "\n";

            std::cout << "Enter sequence file path: ";
            std::string filepath = readLineFromConsole();
            if (filepath.empty()) {
                std::cout << "No file path provided.\n";
                break;
            }

            bool success = false;
            if (key == '1') {
                ed.loadSequencesFromJSON(filepath);
            } else if (key == '2') {
                ed.loadSequencesFromCSV(filepath);
            } else {
                std::cout << "Invalid option.\n";
                break;
            }

            break;
        }
    }
}
