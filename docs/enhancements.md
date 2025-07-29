# Enhancement Plan: NCBI Datasets API Integration

## 1. Objective

This document details the plan to integrate the **NCBI Datasets REST API** into the Alignment Map Viewer. The primary goal is to enable the application to fetch gene and sequence data directly from NCBI's comprehensive databases, transitioning from static, local file-based data to dynamic, live data. This aligns with **Phase 3.2 (Network Integration)** of the main development plan.

**Reference API Documentation:** [NCBI Datasets v2 REST API](https://www.ncbi.nlm.nih.gov/datasets/docs/v2/api/rest-api/)

---
## 2. Scope and Limitations

This integration will be a focused proof-of-concept. To maintain a minimal dependency footprint, **no external HTTP or JSON parsing libraries will be used.** Instead, basic, custom-built functions will be implemented for these tasks.

-   **HTTP Client:** A simple, platform-specific HTTP GET function will be created. It will not support the full range of HTTP features (e.g., HTTPS, various verbs, advanced headers).
-   **JSON Parser:** The parser will be purpose-built to handle the specific structure of the NCBI Gene API response. It will not be a general-purpose JSON parser.
-   **Dataset Access:** The implementation will focus on fetching data for a limited set of genes via their accessions, as a demonstration of capability. It is not intended for large-scale batch processing.

---

## 3. Integration Strategy

The integration will be executed in a phased approach to ensure modularity and testability.

### Phase 1: Core API Logic and Utilities

The first step is to build the foundational logic for making network requests and parsing the results within new `api_logic.h` and `api_logic.cpp` files.

-   **Implement Minimal HTTP GET Function:**
    -   Create a basic function to perform an HTTP GET request to a given URL.
    -   This will likely use platform-specific APIs (e.g., WinINet on Windows) and will be placed within `api_logic.cpp`.
-   **Implement Minimalist JSON Parser:**
    -   Develop a simple string-parsing function tailored to extract specific key-value pairs from the NCBI JSON response for gene data.
-   **Implement Error Handling:**
    -   Handle network-level errors (e.g., connection failed).
    -   Handle HTTP status codes (e.g., `400 Bad Request`, `404 Not Found`, `429 Too Many Requests`).
    -   Parse and report API-specific error messages returned in the JSON body.

### Phase 2: Gene Data Fetching and Mapping

This phase focuses on retrieving gene-specific data.

-   **Target Endpoint:** Use the `/gene/accession/{accessions}` endpoint to fetch data by gene symbol or RefSeq accession.
-   **Implement `fetchGeneData` Method:**
    -   Create a function in `api_logic` that accepts a list of gene identifiers.
    -   This method will call the API and retrieve the gene data package as a JSON object.
-   **Data Mapping (JSON to `GeneModel`):**
    -   The custom JSON parsing logic will directly populate the application's internal `GeneModel` struct.

    | NCBI JSON Field (`gene`) | Application `GeneModel` Field | Notes |
    | -------------------------- | ------------------------------- | ----------------------------------------------------------------------------------------------------------------- |
    | `gene.symbol`              | `gene_name`                     | Direct mapping. |
    | `gene.gene_id`             | `id` (or new field)             | Store the NCBI Gene ID. |
    | `gene.description`         | `description` (or new field)    | Store the full gene name/description. |
    | `gene.genomic_ranges[0]`   | `chromosome`, `start_pos`, `end_pos` | Parse the `accession_version`, and `range[0].begin`, `range[0].end` from the first genomic range. |
    | `gene.type`                | `type` (or new field)           | e.g., "protein-coding". |

    *Note: Fields like `expression_level` and `knockout` are not part of the standard NCBI gene record. This data will need to be sourced from other files or APIs (e.g., GEO for expression) or remain as user-modifiable attributes.*

### Phase 3: User Interface Integration

-   **Add New Input Command:** Introduce a new command in the main view (e.g., `F` for "Fetch") to initiate a network request. This will be handled in `main.cpp`.
-   **Create User Prompt:**
    -   Prompt the user to enter one or more gene symbols or accession numbers.
-   **Provide Feedback:**
    -   Display a "Fetching data from NCBI..." status message to the user.
    -   On success, report "Data loaded for [gene symbols]".
    -   On failure, display the specific error message from the API client.
-   **Load Data into View:**
    -   Clear the existing (or hardcoded) data.
    -   Use the fetched and mapped data to populate the `AlignmentMap`.
    -   Trigger a redraw of the screen to display the new data.

---

## 4. Technical Requirements & Dependencies

1.  **Platform-Specific Network API:** e.g., WinINet on Windows for HTTP requests.
2.  **NCBI API Key (Optional but Recommended):** Users should be able to provide an API key to increase rate limits. The API logic must be able to include this in requests.
3.  **New Modules:**
    -   `api_logic.h/.cpp`: For handling all API communication, including the custom HTTP and JSON parsing logic.