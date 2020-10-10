#include <cstdio>
#include <stdexcept>
#include <fstream>
#include "parse.h"

int main(int argc, char* argv[])
{
	try
	{
		std::string input;
		std::string output = "spooky.asm";
		for (int i = 1; i < argc; i++)
		{
			if (argv[i][0] == '-')
			{
				switch (argv[i][1])
				{
					case 'o':
					{
						if (argv[i][2] != '\0')
						{
							output = argv[i] + 2;
						}
						else
						{
							i++;
							if (i >= argc)
							{
								throw std::runtime_error("invalid usage (\"-o\" as last argument)");
							}
							output = argv[i];
						}
					}
					break;
				}
			}
			else
			{
				input = argv[i];
			}
		}

		std::string spookyFileContents;
		std::fstream spookyFile(input, std::ios_base::in);
		spookyFile.seekg(0, std::ios_base::end);
		const size_t spookyFileSize = spookyFile.tellg();
		spookyFileContents.resize(spookyFileSize);
		spookyFile.seekg(0, std::ios_base::beg);
		spookyFile.read(&spookyFileContents[0], spookyFileSize);

		spookyFile.close();

		std::stringstream opAsm = compile(spookyFileContents);
		
		std::fstream asmFile(output, std::ios_base::out);
		asmFile << opAsm.str();
	}
	catch (const InvalidSyntaxException& e)
	{
		printf("Error: %s (line: %zu, col: %zu)\n", e.what(), e.y, e.x);
		return EXIT_FAILURE;
	}
	catch (const std::exception& e)
	{
		printf("Error: %s\n", e.what());
		return EXIT_FAILURE;
	}
	catch (...)
	{
		puts("Error: unknown error");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}