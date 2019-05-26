local startTime = os.time();

local fileReader = PLP:core():createFileReader("D:/Repositories/LogParser/resources/largeGeneratedFile.txt", 0, true);
local indexWriter = PLP:core():createResultSetWriter("D:/Repositories/LogParser/resources/largeGeneratedFileResults1", 1000000, fileReader, true);

PLP:core():searchMultiline(fileReader,indexWriter,0,0,0,
	{[-1]=PLP.MatchString("9",false,false), [1]=PLP.MatchString("11",false,false)}
);

PLP:core():printConsole("Total time: " .. (os.time() - startTime));