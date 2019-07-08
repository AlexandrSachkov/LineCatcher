local original = "   this\tis a   test string \r\n"; -- contains tab and end line characters
LC:core():printConsole("Original: \"" .. original .. "\"");

local trimmed = LC.stringTrim(original); -- trims all spaces and special characters representing spaces from both ends of the string
LC:core():printConsole("Trimmed:  \"" .. trimmed .. "\"");

local words = LC.splitIntoWords(original); -- removes all spaces and special characters representing spaces from the string and returns words

LC:core():printConsole("Words:");
for i,word in ipairs(words) do
	LC:core():printConsole("\"" .. word .. "\"");
end