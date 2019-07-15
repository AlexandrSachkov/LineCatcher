local fr = LC:core():createFileReader(
	"D:/Repositories/LogParser/data/largeGeneratedFile.txt",
	0
);

local ir = LC:core():createIndexReader(
	"D:/Repositories/LogParser/data/largeGeneratedFileResults1.lcidx",
	0
);

local scanner = LC.LineScannerI(
	fr,
	ir,
	0,
	0
);

local result, lineNum, line = scanner:nextLine();
while result == LC.SUCCESS do
	result, lineNum, line = scanner:nextLine();
	LC:core():printConsole(line);
end