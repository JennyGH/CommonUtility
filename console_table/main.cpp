#include "console_table.hpp"

int main(int argc, char *argv[])
{
	console_table()
		.add_column("NAME").add_column("TYPE").add_column("REQUIRED").add_column("DEFAULT").add_column("DESCRIBTION")
		.add_row("url", "STRING", true, "", "String value.")
		.add_row("pointer", "POINTER", true, nullptr, "Pointer value.")
		.print();

	return 0;
}