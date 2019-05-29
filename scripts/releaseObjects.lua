local startTime = os.time();

local fileWriter = PLP:core():createFileWriter("D:/Repositories/LogParser/resources/test.txt", 0, true);
PLP:core():releaseFileWriter(fileWriter);
local fileWriter2 = PLP:core():createFileWriter("D:/Repositories/LogParser/resources/test.txt", 0, true);
PLP:core():releaseFileWriter(fileWriter2);
local fileReader = PLP:core():createFileReader("D:/Repositories/LogParser/resources/test.txt", 0);
PLP:core():releaseFileReader(fileReader);
local fileReader2 = PLP:core():createFileReader("D:/Repositories/LogParser/resources/test.txt", 0);
local indexWriter = PLP:core():createResultSetWriter("D:/Repositories/LogParser/resources/testIndex.plpidx", 0, fileReader2, true);
PLP:core():releaseIndexWriter(indexWriter);
local indexReader = PLP:core():createResultSetReader("D:/Repositories/LogParser/resources/testIndex.plpidx", 0);



PLP:core():printConsole("Total time: " .. (os.time() - startTime));