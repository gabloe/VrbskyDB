

VrbskyDB Database Management System
===================================

We have implemented a NoSQL database which supports a rich query language.
The language supports CRUD as well as some analytical operations, i.e max, min, average, etc.
We implement a memory-mapped filesystem for efficient disk operations.

Future improvements:
--------------------
We want to be able to scale our database across multiple nodes as well among multiple concurrent users.

Compilation:
-------------------
### On Windows
Open the Visual Studio 2013 solution and build

### On Apple OSX and Linux
cd into the src directory and run make

Interface with User
-------------------
### Insert

**Inserts automatically create a project if it does not exist.**

* INSERT INTO herp WITH { "Hello" : "World" };

* INSERT INTO herp WITH [ { "Hello" : "World" } , {"Hello" : "Moto" } ];	% Insert two records into herp

### Update

UPDATE p_name WITH { json } WHERE { criteria } 

UPDATE herp WITH { "Hello" : "Goodbye" } WHERE { "Hello" : "Moto" } LIMIT 1;

### Delete

DELETE FROM p_name WHERE { criteria } [ LIMIT number ]

DELETE FROM People WHERE { "fName" : "Todd" };

### Create Index

CREATE INDEX ON [ field1 , field2 , ... ];

### Select

SELECT field [, field] IN p_name;
SELECT field [, field] IN p_name WHERE {};

SELECT * FROM People;

SELECT * FROM People WHERE { "fName": "Jerf"};

SELECT * FROM People WHERE { "age" : { "#gt" : 5 } };

SELECT * FROM People WHERE { "spouse" : { "fName" : "Mildred" } };
