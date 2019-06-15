local fileWriter = LC:core():createFileWriter(
	"D:/Repositories/LogParser/scripts/test_data/generated.txt", -- file path
	0,	-- preferred buffer size in bytes (0 means use default value of 64KB)
	true	-- overwrite file if already exists
);

for i=1,1000000 do
	fileWriter:appendLine("abcdefghijklmnopqrstuvwxyz " .. i);	-- writes a million lines to file
end