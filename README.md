# Alignment Map Viewer

A prototype proof-of-concept project demonstrating interactive genomic data visualization and sequence alignment capabilities. This project showcases what may be possible with future technology for processing and visualizing large-scale genomic datasets.

## Overview

The Alignment Map Viewer is a console-based application that provides:
- **3D-style gene mapping visualization** with interactive camera controls
- **Multiple Sequence Alignment (MSA) editor** with real-time editing capabilities
- **Gene statistics and metadata display** with comprehensive genomic information
- **Data import capabilities** for JSON and CSV file formats
- **Interactive gene manipulation** including knockout toggling

## Features

### Core Functionality
- **Interactive 3D Gene Map**: Navigate through genomic data with pan, zoom, and rotation controls
- **Real-time Statistics**: Live calculation of gene expression levels, polygenic scores, and knockout status
- **Sequence Alignment Editor**: Edit DNA/RNA sequences with gap insertion, reverse complement, and base editing
- **Gene Selection**: Browse through genes with detailed information display
- **Knockout Toggle**: Interactive modification of gene knockout status

### Data Import Support
The application supports importing genomic data from multiple file formats:

#### JSON Format Support
- **Gene Data**: Import gene information including expression levels, chromosomal locations, and knockout status
- **Sequence Data**: Load DNA/RNA sequences with annotations and alignment information
- **Flexible Schema**: Handles various JSON structures for different data types

#### CSV Format Support
- **Gene Expression Data**: Import gene names, knockout status, activity levels, and expression measurements
- **Sequence Information**: Load sequence IDs, raw sequences, and annotation data
- **Automatic Parsing**: Intelligent handling of missing values and different column formats

## Installation

### Prerequisites
- **Windows Operating System** (required for console API functionality)
- **C++ Compiler** supporting C++17 or later (Visual Studio 2019+ recommended)
- **Windows SDK** for console and file system APIs

### Building the Project
1. Clone or download the project files
2. Ensure all source files are in the same directory:
   - `main.cpp`
   - `map_logic.h`
   - `map_logic.cpp`
3. Compile using your preferred C++ compiler:
   ```bash
   g++ -std=c++17 main.cpp map_logic.cpp -o alignment_map_viewer.exe
   ```
   Or use Visual Studio to build the project.

## Usage

### Starting the Application
Run the executable from the command line:
```bash
./alignment_map_viewer.exe
```

### Main Interface Controls

#### Gene Map Navigation
- **Arrow Keys (←/→/↑/↓)**: Pan the camera view
- **Q/E**: Rotate the view angle
- **W/S**: Zoom in/out
- **N/P**: Navigate to Next/Previous gene
- **K**: Toggle knockout status of selected gene

#### Mode Switching
- **A**: Enter Alignment Editor mode
- **Esc**: Exit current mode or quit application

### Alignment Editor Controls
- **Arrow Keys (←/→)**: Move cursor position
- **Arrow Keys (↑/↓)**: Select different sequence
- **G**: Toggle gap at cursor position
- **R**: Reverse complement selected sequence
- **E**: Edit base at cursor position (prompts for A/C/G/T/U)
- **Esc**: Return to main gene map view

### Data Import (Future Enhancement)

#### JSON File Format Examples

**Gene Data Format:**
```json
{
  "genes": [
    {
      "gene_name": "GeneA",
      "knockout": true,
      "status": "Inactive",
      "expression_level": 5.2
    },
    {
      "gene_name": "GeneB",
      "knockout": false,
      "status": "Active",
      "expression_level": 8.1
    }
  ]
}
```

**Sequence Data Format:**
```json
{
  "sequences": [
    {
      "sequence_id": "seq1",
      "sequence": "ATGCGT--A--C",
      "annotations": "Start codon present"
    },
    {
      "sequence_id": "seq2",
      "sequence": "ATG--GTA--C",
      "annotations": "Gap present"
    }
  ]
}
```

#### CSV File Format Examples

**Gene Expression Data:**
```csv
gene_name,knockout,status,expression_level
GeneA,X,Inactive,5.2
GeneB,,Active,8.1
GeneC,X,Inactive,3.4
GeneD,,Active,7.0
```

**Sequence Data:**
```csv
sequence_id,sequence,annotations
seq1,ATGCGT--A--C,Start codon present
seq2,ATG--GTA--C,Gap present
seq3,ATGCGTACGT,No gaps
seq4,ATGCGTACG-,End codon present
```

## Display Information

### Gene Map View
- **3D Visualization**: Genes displayed as points in pseudo-3D space
- **Gene Markers**:
  - `*` = Normal gene
  - `X` = Knockout gene
- **Statistics Panel**: Shows real-time genomic statistics
- **Gene Details**: Displays selected gene information including:
  - Gene symbol and chromosome location
  - Expression level and polygenic score
  - Knockout status
  - Genomic coordinates (start-end positions)

### Alignment Editor View
- **Reference Sequence**: Top line showing the reference alignment
- **Multiple Sequences**: Individual sequences with alignment gaps
- **Cursor Position**: Highlighted base position for editing
- **Visual Indicators**:
  - `>` marks the currently selected sequence
  - `[ ]` brackets show cursor position

## Technical Details

### Architecture
- **Modular Design**: Separate header and implementation files
- **Real-time Rendering**: Console-based display with immediate updates
- **Memory Efficient**: In-memory data structures optimized for interactive use
- **Cross-platform Ready**: Core logic separable from Windows-specific display code

### Data Structures
- **GeneModel**: Stores comprehensive gene information
- **SequenceModel**: Manages DNA/RNA sequence data with alignment
- **AlignmentMap**: Container for gene collections with statistics
- **AlignmentEditor**: Handles sequence editing operations

### Performance Considerations
- **Efficient Rendering**: Optimized console output for smooth interaction
- **Memory Management**: Careful handling of large genomic datasets
- **Real-time Updates**: Immediate response to user input

## Future Enhancements

### Planned Features
1. **File Import Integration**: Direct loading of JSON and CSV files
2. **Data Validation**: Comprehensive error checking and parsing feedback
3. **Export Capabilities**: Save modified data to various formats
4. **Advanced Visualization**: Enhanced 3D rendering with OpenGL
5. **Network Integration**: Remote data source connectivity
6. **Multi-platform Support**: Linux and macOS compatibility

### Data Processing Improvements
- **Large File Handling**: Streaming support for massive datasets
- **Format Auto-detection**: Automatic identification of file types
- **Schema Validation**: Robust parsing with detailed error reporting
- **Batch Processing**: Multiple file import capabilities

## Limitations

### Current Restrictions
- **Windows Only**: Console API dependency limits platform compatibility
- **Demo Data**: Currently uses hardcoded sample data
- **File Import**: Manual integration required for external data sources
- **Memory Constraints**: Large datasets may require optimization

### Known Issues
- **Console Display**: Limited to text-based visualization
- **Input Handling**: Windows-specific keyboard event processing
- **Data Persistence**: No save/load functionality for session data

## Contributing

This is a prototype project demonstrating proof-of-concept functionality. Future development may include:
- Cross-platform compatibility layers
- Advanced data import/export capabilities
- Enhanced visualization options
- Performance optimizations for large datasets

## License

This project is a technological demonstration and proof of concept. Please refer to your organization's licensing requirements for usage and distribution.

## Support

For technical questions or enhancement requests, please refer to the project documentation or contact the development team.

---

*Note: This is a prototype system demonstrating potential future capabilities. Current technological limitations may restrict full implementation of all described features.*

