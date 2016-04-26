#ifndef BPRINTER_TABLE_PRINTER_H_
#define BPRINTER_TABLE_PRINTER_H_

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

namespace bprinter {
	class endl {};

	class TablePrinter {
	public:
		TablePrinter(std::ostream * output, const std::string & separator = "|");
		~TablePrinter();

		int get_num_columns() const;
		int get_table_width() const;
		void set_separator(const std::string & separator);

		void AddColumn(const std::string & header_name, int column_width);
		void PrintHeader();
		void PrintFooter();

		TablePrinter& operator<<(endl input) {
			while (j_ != 0) {
				*this << "";
			}
			return *this;
		}

		TablePrinter& operator<<(float input);
		TablePrinter& operator<<(double input);

		template<typename T> TablePrinter& operator<<(T input) {
			if (j_ == 0)
				*out_stream_ << "|";
						
			*out_stream_ << std::setw(column_widths_.at(j_))
				<< input;

			if (j_ == get_num_columns() - 1) {
				*out_stream_ << "|\n";
				i_ = i_ + 1;
				j_ = 0;
			}
			else {
				*out_stream_ << separator_;
				j_ = j_ + 1;
			}

			return *this;
		}

	private:
		void PrintHorizontalLine();

		template<typename T> void OutputDecimalNumber(T input);

		std::ostream * out_stream_;
		std::vector<std::string> column_headers_;
		std::vector<int> column_widths_;
		std::string separator_;

		int i_;
		int j_;

		int table_width_;
	};

}

#include "table_printer.tpp.h"
#endif