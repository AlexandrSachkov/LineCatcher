local startTime = os.time();

local fileReader = LC:core():createFileReader("D:/Repositories/LogParser/data/ShadowOfTombRaider_Vega_18.12.2_RS4_2_SOTTR_64_4b1d4982_1_11950.dump", 0);

LC:core():printConsole("Total time: " .. (os.time() - startTime));