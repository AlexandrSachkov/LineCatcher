-- open file
local fileReader = LC:core():createFileReader(
	"D:/Repositories/LogParser/data/test_data/lorem-ipsum.txt", -- file path
	 0	-- preferred buffer size (0 means use default size of 64 KB)
);

-- create index writer
local indexWriter = LC:core():createIndexWriter(
	"D:/Repositories/LogParser/data/test_data/lorem-ipsum-index2.lcidx", -- index path
	0, -- use default buffer size
	fileReader, -- file reader for source file
	true -- overwrite if exists
);


-- define search criteria

-- create custom function that rejects lines containing "Suspendisse"
function notContains(line)
	if line:find("Suspendisse", 0, true) then -- returns false if line contains "Suspendisse"
		return false;
	end
	return true;
end

local criteria = LC.MatchAll({ -- match all the following criteria
	LC.MatchString("lorem", false, true), -- line contains "lorem"
	LC.MatchRegex("ipsum", false), -- line matches regular expression (contains "ipsum")
	LC.MatchCustom(notContains) -- line matches custom criteria defined by our function (line does not contain "Suspendisse")
	-- see Lua API docs for more comparator types
});

--read file one line at a time
local line = "";
local result;

while true do
	result, line = fileReader:nextLine(); --read line
	if result ~= LC.SUCCESS then	-- stop if failed to read line
		break
	end

	if criteria:match(line) then -- index all lines matching our criteria
		indexWriter:appendCurrentLine(fileReader); -- adds index for current line
	end
end