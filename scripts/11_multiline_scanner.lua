local fr = LC:core():createFileReader(
	"D:/Repositories/LogParser/data/largeGeneratedFile.txt",
	0
);

local ir = LC:core():createIndexReader(
	"D:/Repositories/LogParser/data/largeGeneratedFileResults1.lcidx",
	0
);

local scanner = LC.MultilineScannerI(
	fr,
	ir,
	-1, -- defines lower bound of the frame relative to its center (line 0)
	1,	-- defines upper bound of the frame relative to its center
	0,	-- starting line
	0	-- end line
);

local result = scanner:nextFrame();
local line1, line2, line3;
while result == LC.SUCCESS do
	_,_,line1 = scanner:getLine(-1);
	_,_,line2 = scanner:getLine(0);
	_,_,line3 = scanner:getLine(1);
	LC:core():printConsole(line1);
	
	result = scanner:nextFrame();
end