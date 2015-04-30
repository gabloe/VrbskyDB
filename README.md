

VrbskyDB NoSQL Database System Project
======================================

We have implemented a NoSQL database which supports a rich query language.
The language supports CRUD as well as some analytical operations, i.e max, min, average, etc.
We implement a memory-mapped filesystem for efficient disk operations.

Future improvements:
--------------------
We want to be able to scale our database across multiple nodes as well among multiple concurrent users.


Interface with User
-------------------

### Query Language
#### Create a project and/or document
<ul>
<li>create document <b>PNAME</b>.<b>DNAME</b>;</li>
<li>create document <b>PNAME</b>.<b>DNAME</b> with value <b>JSON</b>;</li>
<li>create project <b>PNAME</b>;</li>
<li>create project <b>PNAME</b> with documents (<b>DNAME_1</b>, <b>DNAME_2</b>, ... , <b>DNAME_N</b>);</li>
</ul>

#### Show available projects or documents
<ul>
<li>show projects;</li>
<li>show documents in <b>PNAME</b>;</li>
</ul>

#### Delete a document or project
<ul>
<li>delete project <b>PNAME</b>;</li>
<li>delete document <b>PNAME</b>.<b>DNAME</b>;</li>
</ul>

Under Construction!

Data Retrieval
-------------------

Operations on Data
-------------------

Load balancing
-------------------

Data storage
-------------------

