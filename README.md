# Simly-DataBase
# Description
This C program implements a lightweight database for managing vehicle inspection records. Records are stored in a linked list (queue) and contain information such as unit ID, model, registration number, inspection date, status, mechanic, and driver. The program processes commands from an input file (input.txt), performs operations like insert, select, delete, update, remove duplicates (uniq), and sort, and writes the results to output.txt. Memory allocation statistics are saved to memstat.txt.

# Features
Record storage – each record includes:

unit_id (integer)

unit_model (string in double quotes)

car_id (registration number in a specific format: 'A123BC123' – letters from a defined set, digits)

chk_date (inspection date in 'dd.mm.yyyy' format)

status (one of: well, wearlow, wearhigh, broken, notcheck)

mechanic (string in double quotes)

driver (string in double quotes)

Commands (each line in input.txt):

insert field=value, field=value, ... – adds a new record.

select field1,field2,... [condition ...] – displays selected fields for records matching optional conditions.

delete condition ... – removes records satisfying the conditions.

update field=value,field=value,... [condition ...] – modifies records that meet the conditions.

uniq field1,field2,... – removes duplicate records based on the specified fields (keeps the last occurrence).

sort field1=asc/desc, field2=asc/desc,... – sorts the list by the given fields (status field cannot be used for sorting).

Conditions – support operators:

Comparison: ==, !=, <, >, <=, >= for numeric, string, date, and car number fields.

Set membership: /in/ [value1,value2,...] and /not_in/ [...] (mainly for status field).

Input validation – all field values are checked against their expected formats; invalid commands produce incorrect:'<truncated line>' in output.

Memory tracking – counts malloc, realloc, free, and strdup calls; writes statistics to memstat.txt.

Dynamic line reading – input lines are read with a growing buffer, supporting long commands.

# Requirements
C compiler with C99 support (e.g., GCC, MSVC).

Standard C library.

Installation
Clone the repository or download the source file lab_db.c.

Compile the program using a C compiler. Example with GCC:

bash
gcc -o lab_db lab_db.c -std=c99
Prepare an input.txt file with the desired commands (see examples below).

Run the program:

bash
./lab_db
Results will be written to output.txt and memory statistics to memstat.txt.

# Usage
Input file format
Each line in input.txt contains one command. Spaces are allowed but not required. Field names are case‑sensitive and must match exactly.

Example input.txt:
text
insert unit_id=1 unit_model="KamAZ" car_id='A123BC123' chk_date='15.03.2025' status=well mechanic="Ivanov" driver="Petrov"
insert unit_id=2 unit_model="GAZ" car_id='B456AB78' chk_date='20.03.2025' status=broken mechanic="Sidorov" driver="Ivanov"
select unit_id,driver status==well
update status='wearlow' unit_id==2
sort unit_id=asc,chk_date=desc
uniq driver,mechanic
Output format
For insert: insert:<new_queue_size>

For select: first line select:<count>, then for each matching record the requested fields separated by spaces.

For delete: delete:<deleted_count>

For update: update:<updated_count>

For uniq: uniq:<removed_duplicates_count>

For sort: sort:<queue_size>

If a command is malformed: incorrect:'<first 20 chars of the line>'

Field formats
unit_id – integer.

unit_model – string enclosed in double quotes, e.g. "KamAZ".

car_id – string enclosed in single quotes, format: letter (from ABCEHKMOPTXY), three digits, two letters, two or three digits (region). Example: 'A123BC123'.

chk_date – date in single quotes: 'dd.mm.yyyy'. Year between 1000 and 2026, valid day for the month.

status – one of the following (without quotes): well, wearlow, wearhigh, broken, notcheck. In conditions, can be a list: [well,broken].

mechanic – string in double quotes.

driver – string in double quotes.

Condition examples
unit_id==5

unit_model!="KamAZ"

chk_date>='01.01.2025'

status /in/ [well,broken]

status /not_in/ [wearlow,wearhigh]

car_id<'B000AB50' (comparison is performed by digit parts and letters according to the format)

# Notes
The uniq command removes duplicates based on the specified fields, keeping only the last occurrence of each unique combination (the list is reversed before processing).

The sort command cannot use the status field as a sort key.

All dynamic memory is tracked and freed; no leaks should remain after normal exit.

The program counts strdup calls separately from malloc.

# License
This project is provided for educational purposes. No explicit license is specified. The code may be used and modified at your own risk.


