-- open file
local fileReader = LC:core():createFileReader(
	"D:/Repositories/LogParser/data/test_data/lorem-ipsum.txt", -- file path
	 0	-- preferred buffer size (0 means use default size of 64 KB)
);

-- create index file for search results
local inderWriter = LC:core():createIndexWriter(
	"D:/Repositories/LogParser/data/test_data/lorem-ipsum-results1.plpidx",
	0,
	fileReader,
	true
);

-- create text comparator that will find us lines containing the word "ipsum"
local comparator = LC.MatchString(
	"ipsum", -- string to search for
	false	 -- do not use exact matching
);

local result = LC:core():search(
	fileReader,
	inderWriter,
	0,	-- line where search will start
	0,	-- line where search will end (0 means search every line)
	0,	-- maximum number of results to find (0 means find all results)
	comparator
);

if result == true then
	LC:core():printConsole("SUCCESS");
else
	LC:core():printConsole("FAILED");
end