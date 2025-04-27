# Database Project

This is a lightweight relational database that efficiently retrieve key value storage. The database manages user-saved places, using a custom-built **B+ Tree** storage engine, **secondary indexing**, and **write-ahead logging** for crash recovery.
It also integrates with **Google Maps** via Python scripts to import real-world "Saved Places" data into the database.

# Project Milestone 1

Implemented the B Plus Tree Algorithm in BPlusTreeFull.cpp and defining the skeleton structure of the BPlusTree and node files.

---

## Project Components

**BPlusNode**: Represents a node (internal or leaf) in a B+ Tree. Handles splitting, merging, and traversal.
**BPlusTree**: Manages the B+ Tree structure. Provides insert, delete, search, range search, prefix search operations.
**BTreeIndex**: A wrapper around the B+ Tree, abstracting the interface for database use.
**LeaderDB**: Main database engine. Inherits publicly from DBInstance abstract class. The LeaderDB manages tables (with indexes), and executes CRUD operations, handles WAL (Write-Ahead Logging) for recovery.
**SecondaryIndex**: Supports secondary indexes (non-primary keys) for faster attribute-based search (e.g., find by email, find by title).
**WriteAheadLog**: Ensures database durability by logging changes before applying them to disk. Supports crash recovery.
**CsvParser / FileUtils / DataInserter**: Utilities for loading CSV files into tables and inserting structured place data.

## Database Schema

The database schema supports users importing their saved places lists from Google Maps:

### 1. Users Table (`users`)

- `email` (PK): Email address (used as key).
- `user_id`: Unique generated integer ID.
- `created_at`: Account creation timestamp.

### 2. Places Table (`places`)

- `place_id` (PK): Google Place ID.
- `name`: Name of the place (e.g., "Central Park").
- `address`: Formatted address.
- `latitude`: Latitude coordinate.
- `longitude`: Longitude coordinate.

### 3. SavedLists Table (`savedlists`)

- `list_id` (PK): Random unique list ID.
- `user_id` (FK): Belongs to a user.
- `title`: Title of the list (e.g., "Favorites").

### 4. ListPlaces Table (`listplaces`)

- Composite Primary Key: (`list_id`, `place_id`)
- Links places to lists (many-to-many relationship).

---

## Building the Project

Make sure you have a C++17 compatible compiler installed.

1. Clone the repository.
2. In the project root, compile the program:

```bash
make clean
make
```

---

### To Upload Google Takeout File

#### 1. Get Google API Key and keep in a .env file

#### 2. Create and activate Python virtual environment

#### 3. Install dependencies

Run

```bash
pip install -r requirements.txt
```

#### 4. Get your Google takeout data and drop them in the takeout/Saved folder

#### 5. Run Python script:

```bash
python fetch_places.py takeout/Saved <user email>
```
