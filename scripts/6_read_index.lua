local fileReader = LC:core():createFileReader(
	"D:/Repositories/LogParser/data/largeGeneratedFile.txt", -- file path
	0   -- use default buffer size
)

local indexReader = LC:core():createIndexReader(
	"D:/Repositories/LogParser/data/largeGeneratedFileResults1.txt", -- index path
	0 -- use default buffer size
);

--read index one line at a time
local lineNumber;
local indexReadResult;
local fileReadResult;
local line;

while true do
	indexReadResult, lineNumber = indexReader:nextIndex(); -- read index
	if indexReadResult ~= true then	-- stop if failed to read index
		break
	end

	fileReadResult, line = fileReader:getLineFromIndex(indexReader); -- get line from current index
	if fileReadResult ~= LC.SUCCESS then	 -- stop if failed to read file
		break
	end

	LC:core():printConsole(line);	-- print line to console
end