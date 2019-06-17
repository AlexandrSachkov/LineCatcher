-- open file
local fileReader = LC:core():createFileReader(
	"D:/Repositories/LogParser/data/test_data/lorem-ipsum.txt", -- file path
	 0	-- preferred buffer size (0 means use default size of 64 KB)
);

local indexWriter = LC:core():createIndexWriter(
	"D:/Repositories/LogParser/data/test_data/lorem-ipsum-index.lcidx", -- index path
	0, -- use default buffer size
	fileReader, -- file reader for source file
	true -- overwrite if exists
);

--read file one line at a time
local line = "";
local result;

while true do
	result, line = fileReader:nextLine(); --read line
	if result ~= LC.SUCCESS then	-- stop if failed to read line
		break
	end

	if line:find("ipsum", 0, true) then -- checks if line contains "ipsum". This is a standard Lua string operation (http://lua-users.org/wiki/StringLibraryTutorial)
		indexWriter:appendCurrentLine(fileReader); -- adds index for current line
	end
end