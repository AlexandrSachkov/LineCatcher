local startTime = os.time();

local fileReader = LC:core():createFileReader("D:/Repositories/LogParser/data/largeGeneratedFile.txt", 0);
local indexWriter = LC:core():createIndexWriter("D:/Repositories/LogParser/data/largeGeneratedFileResults1", 1000000, fileReader, true);

LC:core():searchMultiline(fileReader,indexWriter,0,0,0,
	{[-1]=LC.MatchString("9",false,false), [1]=LC.MatchString("11",false,false)}
);

LC:core():printConsole("Total time: " .. (os.time() - startTime));