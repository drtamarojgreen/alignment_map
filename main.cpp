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
struct UIState  { int geneIdx=0; Camera cam; bool inAlign=false; };

// Forward prototypes
void initConsole();
void clearScreen();
void drawMap(const AlignmentMap&, UIState&);
void drawStats(const AlignmentMap&, UIState&);
void drawAlignment(AlignmentEditor&, UIState&);
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
            if (vk==VK_ESCAPE) {
                if (st.inAlign) st.inAlign = false;
                else break;
            }
            else if (vk=='A' && !st.inAlign) {
                st.inAlign = true;
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
        } else {
            drawMap(map, st);
            drawStats(map, st);
            // show mode hint
            COORD p{0, SHORT(SCREEN_H-1)};
            SetConsoleCursorPosition(hOut, p);
            std::cout << "[A]Align  [L]Load  [N/P]Next/Prev Gene  [←/→/↑/↓]Pan/Rotate/Zoom  [Esc]Quit";
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

    for (int i=0;i<STAT_H;++i) {
        COORD p{0, SHORT(MAP_H + i)};
        SetConsoleCursorPosition(hOut,p);
        switch(i) {
          case 0: std::cout<<"Stats:"; break;
          case 1: std::cout<<" Total:"   <<stats.totalGenes; break;
          case 2: std::cout<<" KO:"      <<stats.totalKnockouts; break;
          case 3: std::cout<<" Expr:"    <<stats.avgExpression; break;
          case 4: std::cout<<" Poly:"    <<stats.avgPolyScore; break;
          case 5: std::cout<<" Updated:"<<stats.timestamp; break;
          case 7: std::cout<<" Gene:"   <<g.symbol; break;
          case 8: std::cout<<" Chr:"    <<g.chromosome<<" ["<<g.start<<"-"<<g.end<<"]"; break;
          case 9: std::cout<<" ExpLvl:" <<g.expressionLevel; break;
          case 10:std::cout<<" PScore:" <<g.polygenicScore; break;
          case 11:std::cout<<" KO?:"    <<(g.isKnockout?"YES":"no"); break;
        }
    }
}

// draw alignment editor
void drawAlignment(AlignmentEditor& editor, UIState& st) {
    editor.render(SCREEN_W, SCREEN_H);
}

void handleMainKey(int vk, AlignmentMap& map, UIState& st) {
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
