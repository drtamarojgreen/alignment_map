# Development Plan: Alignment Map Viewer

## 1. Objective

This document outlines the development roadmap to evolve the Alignment Map Viewer from a proof-of-concept prototype into a robust, feature-rich, and cross-platform genomic data visualization tool. The plan is divided into distinct phases, prioritizing foundational improvements before moving on to advanced features.

---

## Phase 1: Foundational Improvements & Core I/O

This phase focuses on addressing the most significant limitations of the current prototype: hardcoded data and platform dependency.

### 1.1. File Import and Export
-   **Implement JSON Data Importer**:
    -   Develop a robust parser for the specified gene and sequence JSON formats.
    -   Integrate the parser to load data at application startup via a command-line argument.
-   **Implement CSV Data Importer**:
    -   Develop a parser for gene expression and sequence CSV formats.
    -   Handle variations in data, such as different markers for knockout status (`X`, `true`, `1`).
-   **Implement Data Export**:
    -   Create functionality to save the current state of genes and sequences (including edits) back to a JSON file.
    -   Ensure that user modifications from the Alignment Editor and main view (e.g., knockout toggling) are persisted.

### 1.2. Data Validation
-   **Schema Validation**: Implement checks to ensure imported files conform to the expected structure.
-   **Error Reporting**: Provide clear, user-friendly error messages for parsing failures or data inconsistencies.

### 1.3. Refactor for Cross-Platform Support
-   **Abstract Console I/O**: Isolate all Windows-specific API calls (`Windows.h`, `conio.h`) into a separate abstraction layer.
-   **Create Platform-Agnostic Interface**: Define a common interface for screen drawing, color handling, and keyboard input.
-   **Initial Linux/macOS Port**: Implement the abstraction layer for a POSIX-compliant terminal (using ncurses or similar) to enable compilation and basic functionality on other platforms.

---

## Phase 2: Advanced Data Handling and Usability

With foundational I/O in place, this phase will improve the application's ability to handle large-scale data and enhance the user experience.

### 2.1. Large-Scale Data Processing
-   **Streaming Data Support**: Modify file importers to read data in chunks (streams) rather than loading entire files into memory. This is critical for handling massive genomic datasets.
-   **Format Auto-detection**: Implement logic to automatically detect whether an input file is JSON or CSV, simplifying the user workflow.
-   **Batch Processing**: Allow the application to accept multiple files or a directory as input for batch import operations.

### 2.2. User Experience Enhancements
-   **Improved In-App Feedback**: Display status messages for successful imports, saves, and errors directly in the UI.
-   **Configuration File**: Allow users to set default paths or preferences via a configuration file.

---

## Phase 3: Advanced Visualization and Connectivity

This phase focuses on long-term goals that will transform the application into a professional-grade tool.

### 3.1. Graphical Rendering Engine
-   **Integrate Graphics Library**: Replace the console-based rendering with a true graphical library like OpenGL (or a C++ wrapper like SFML/SDL).
-   **Enhance 3D Visualization**: Implement a true 3D camera, lighting, and more sophisticated graphical representations for genes.
-   **UI Framework**: Introduce a simple UI framework for buttons, menus, and file dialogs to replace the keyboard-only interface.

### 3.2. Network Integration
-   **Remote Data Sources**: Design and implement a module to fetch genomic data from remote APIs or databases.
-   **Standardized API Client**: Create a client for interacting with common genomic data repositories (e.g., Ensembl, NCBI).

---

## Summary of Key Milestones

1.  **Milestone 1 (End of Phase 1)**: Application can load and save data from JSON/CSV files and runs on at least two operating systems (Windows and Linux).
2.  **Milestone 2 (End of Phase 2)**: Application can efficiently handle large datasets (>1GB) and provides a more robust user experience with better error handling.
3.  **Milestone 3 (End of Phase 3)**: Application features a full graphical user interface, replacing the console output, and has initial capabilities for network data retrieval.