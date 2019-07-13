local fileReader = LC:core():createFileReader("D:/Repositories/LogParser/data/largeGeneratedFile.txt", 0);

local indexWriter = LC:core():createIndexWriter(
	"D:/Repositories/LogParser/data/largeGeneratedFileResults1.lcidx", 
	1000000, -- use 1MB buffer
	fileReader, 
	true	--overwrite if exists
);

local comparators = {
	[-1]=LC.MatchString("9",false), -- previous line must contain "9"
	[1]=LC.MatchString("11",false)	 -- next line must contain "11"
}

local result = LC:core():searchMultiline(
	fileReader,
	indexWriter,
	0, -- start on the first line
	0, -- search every line
	0, -- retrieve all found results
	comparators
);

if result == true then
	LC:core():printConsole("SUCCESS");
else
	LC:core():printConsole("FAILED");
end