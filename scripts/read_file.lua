-- open file
local fileReader = LC:core():createFileReader(
	"D:/Repositories/LogParser/scripts/test_data/lorem-ipsum.txt", -- file path
	 0	-- preferred buffer size (0 means use default size of 64 KB)
);

-- print number of lines to console
LC:core():printConsole("Num lines: " .. fileReader:getNumberOfLines());

--read file one line at a time
local line = "";
local result;

while true do
	result, line = fileReader:nextLine(); --read line
	if result ~= LC.SUCCESS then	-- stop if failed to read line
		break
	end

	LC:core():printConsole(line);	-- print line to console
end