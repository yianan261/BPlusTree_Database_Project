# Database Project

This is a simulated relational database that efficiently retrieve key value storage. The database manages user-saved places, using a custom-built **B+ Tree** storage engine, and **secondary indexing**.
It also integrates with **Google Maps** via Python scripts to import real-world "Saved Places" data into the database.

---

## Project Components

- **BPlusNode**: Represents a node (internal or leaf) in a B+ Tree. Handles splitting, merging, and traversal.
- **BPlusTree**: Manages the B+ Tree structure. Provides insert, delete, search, range search, and prefix search operations.
- **BTreeIndex**: A wrapper around the B+ Tree, abstracting the interface for database use.
- **LeaderDB**: Main database engine. Inherits publicly from DBInstance abstract class. Manages tables (with indexes), executes CRUD operations.
- **SecondaryIndex**: Supports secondary indexes (non-primary keys) for faster attribute-based search (e.g., find by email, find by title).
- **CsvParser / FileUtils / DataInserter**: Utilities for loading CSV files into tables and inserting structured place data.
  
### Note
Currently Write Ahead Log implementations are incomplete, please disregard at the current time of project. We are keeping it to continue working on it later.

## Inheritance and Polymorphism

1. InternalNode and LeafNode inherit publicly from base class BPlusNode
2. LeaderDB inherits publicly from abstract base class DBInstance. This class could extend to future DB nodes for multiple DB instances.

## Database Schema

The database schema supports users importing their saved places lists from Google Maps:

### 1. Users Table (`users`)

- `email`: Email address (used as key).
- `user_id`(PK): Random unique generated integer ID.
- `created_at`: creation timestamp.

### 2. Places Table (`places`)

- `hashedId` (PK): The Google place_id is hashed to integers as the primary key, and the original place id will be kept as a string attribute.
- `place_id`: original place id kept as a string attribute.
- `name`: Name of the place (e.g., "Central Park").
- `address`: Formatted address.
- `latitude`: Latitude coordinate.
- `longitude`: Longitude coordinate.
- `description`: Description of the place.

### 3. SavedLists Table (`savedlists`)
- `list_id` (PK): Random unique list ID.
- `user_id` (FK): Belongs to a user.
- `title`: Title of the list (e.g., "Favorites").
- `createdAt`: timestamp.

### 4. ListPlaces Table (`listplaces`)

- Combined Primary Key of `list_id` and `place_id`: int IDs
- `list_id`: Mapped to `list_id`
- `place_Id`: Links places to lists (many-to-many relationship)

---

## Commands (Database Menu Options)

1. `get <key>`
   Retrieve one record (all attributes) from the current table using its primary key.

```bash
get <key>
```

2. `create`
   Create a new record or a new table. You will be prompted to specify if you want to create a table or a record.

```bash
create
// Program will ask if you want to create table or instance (record in a table)
(table/instance)
Create table or instance? table
Enter table name: Pets
Enter headers (comma separated): name,age
Table pets created with headers: name,  age
```

3. `update`
   Update an existing record in the current table by specifying a key and providing new attributes.

```bash
update
Key: (Enter key)
Attributes (comma separated): (Enter comma separated attributes)
Record updated.
```

4. `delete <key>`
   Delete a specific record from the current table using its primary key.

```bash
delete:
Enter key to delete: 5
Key removed.
```

5. `drop (table)`
   Delete an entire table from the database (except the default table).

 ```bash
 drop
 Please enter table name to drop: places
 Table dropped.
 ```
6. `use (table)`
   Switch to a different table for subsequent operations.

```bash
use (tablename)

```
7. `tables`
 List all existing tables in the database.

8. `load <filepath>`
   Load records from a CSV file into the current table. The first line of the CSV should contain column headers.

```bash
load
Enter CSV file path: (enter path)
```

9. `save`
   Save all tables into CSV files. Each table will be saved to its own CSV under the output/ directory.

```bash
save
Saved default to ./output/default.csv
```

10. `view`
    View up to 10 records from the current table, formatted nicely in columns.

```bash
view
```

11. `createindex <col>`
    Build a secondary index on a specified column to speed up select queries.

```bash
createindex 2
Enter column name: (column name)
Secondary index built on <column name>
```

12. `select <cols>|\* where <col>=<val>`
    Query the current table by applying a filter condition (where) and optionally projecting specific columns.

```bash
select * where name=(name)
```

13. `createuser`
    Create a new user account and optionally upload Google Maps Saved Places data into your database.

```bash
createuser
Enter user email: (email)
Would you like to upload your Saved Places? (yes/no):
```

14. `join`
    Perform a join between two tables based on matching column values, with optional projection of columns.

```bash
join A.1 B.2
```

15. `help`
    Display the full list of available commands and their descriptions.

```bash
help
```

16. `exit`
    Exit the program.

```bash
exit
```

---

## Building the Project

Make sure you have a C++17 compatible compiler installed.

1. Clone the repository.
2. In the project root, compile the program:

```bash
make clean
make
```

### To Run the project

After compiling, run

```bash
./leaderdb
```

### To Test with createuser uploads
Users may use the provided csv files in the filepath `saved_places_dir/yian261_at_gmail_dot_com`
<br>
The `dataset_project` files can be used for other table command testing

---

### To Upload Google Takeout File (Optional)

#### 1. Get Google API Key and keep in a .env file

```bash
GOOGLE_API_KEY=<your api key>
```

#### 2. Create and activate Python virtual environment

```bash
python -m venv myenv
source myenv/bin/activate  # windows `myenv\Scripts\activate`
pip install -r requirements.txt
```

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

---

## Credits

### Authors:

Zirui Wen, Yian Chen

### Dataset:

Google takeout data from Yian Chen

### ASCII Art:

From [Patorjk](http://patorjk.com/software/taag/)
