#Grammar


## Insert

**Inserts automatically create a project if it does not exist.**

INSERT INTO herp WITH { "Hello" : "World" };

INSERT INTO herp WITH [ { "Hello" : "World" } , {"Hello" : "Moto" } ];	% Insert two records into herp


## Update

UPDATE p_name WITH { json } WHERE { criteria } 

UPDATE herp WITH { "Hello" : "Goodbye" } WHERE { "Hello" : "Moto" } LIMIT 1;

## Delete

DELETE FROM p_name WHERE { criteria } [ LIMIT number ]

DELETE FROM People WHERE { "fName" : "Todd" };

## Create Index

CREATE INDEX ON [ field1 , field2 , ... ];

## Find

SELECT field [, field] IN p_name;
SELECT field [, field] IN p_name WHERE {};

SELECT * FROM People;

SELECT * FROM People WHERE { "fName": "Jerf"};

SELECT * FROM People WHERE { "age" : { "#gt" : 5 } };

SELECT * FROM People WHERE { "spouse" : { "fName" : "Mildred" } };



# Implementation

# Data Storage

The JSON data is chunked and stored in a flat file.

