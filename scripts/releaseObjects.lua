local startTime = os.time();

local fileWriter = LC:core():createFileWriter("D:/Repositories/LogParser/resources/test.txt", 0, true);
LC:core():releaseFileWriter(fileWriter);
local fileWriter2 = LC:core():createFileWriter("D:/Repositories/LogParser/resources/test.txt", 0, true);
LC:core():releaseFileWriter(fileWriter2);
local fileReader = LC:core():createFileReader("D:/Repositories/LogParser/resources/test.txt", 0);
LC:core():releaseFileReader(fileReader);
local fileReader2 = LC:core():createFileReader("D:/Repositories/LogParser/resources/test.txt", 0);
local indexWriter = LC:core():createIndexWriter("D:/Repositories/LogParser/resources/testIndex.LCidx", 0, fileReader2, true);
LC:core():releaseIndexWriter(indexWriter);
local indexReader = LC:core():createIndexReader("D:/Repositories/LogParser/resources/testIndex.LCidx", 0);



LC:core():printConsole("Total time: " .. (os.time() - startTime));