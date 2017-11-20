#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <algorithm>

std::map<std::string, std::string> vars; //list for all variables
std::map<std::string, std::map<std::string, std::string> > funcs; //list for all functions
std::vector<std::string> warnings; //all warnings
std::vector<std::string> errors; //all errors
std::string variable_scope = ""; //the variable scope (empty = gobal)
std::vector<char> variable_chars = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '_'}; //all chars that can be in a variable/function name

//pre-define
std::string solve_bool(std::string to_solve);
std::string solve(std::string to_solve);
bool is_float(std::string string);
std::string get_var(std::string name);
bool is_var(std::string name);
std::string raise(std::string text, std::string line, std::string type);
std::string func_lex(std::string line, bool debug_mode);
std::string line(std::string line, int line_num, bool debug_mode);
std::string bool_to_bstring(bool value);
char get_type(std::string var);

bool debug = false;

char peek(const char * & expression) {
	return *expression;
}

char get(const char * & expression) {
	return *expression++;
}

std::string calculate(const char * & expression);

std::string number(const char * & expression) {
	if (debug) std::cout << "number()" << std::endl; 
	std::string result;
	result += get(expression);
	while (peek(expression) >= '0' && peek(expression) <= '9') {
		result += get(expression);
	}
	return "i" + result;
}

std::string string(const char * & expression) {
	if (debug) std::cout << "string()" << std::endl; 
	std::string result;
	get(expression);
	while (peek(expression) != '"') {
		result += get(expression);
	}
	get(expression); // '"'
	return "s" + result;
}

std::string variable(const char * & expression) {
	if (debug) std::cout << "variable()" << std::endl; 
	std::string result;
	std::vector<char> ops = {'+', '-', '*', '/', '(', ')', '='};
	while (std::find(ops.begin(), ops.end(), peek(expression)) == ops.end() and peek(expression) != 0) {
		result += get(expression);
	}
	if (debug) std::cout << result << std::endl;
	return result;
}

std::string factor(const char * & expression) {
	if (debug) std::cout << "factor()" << std::endl;
	if (peek(expression) >= '0' && peek(expression) <= '9') {
		return number(expression);
	}
	else if (peek(expression) == '"') {
		return string(expression);
	}
	else if (peek(expression) == '(') {
		get(expression); // '('
		std::string result = calculate(expression);
		get(expression); // ')'
		return result;
	}
	else if (peek(expression) == '-') {
		get(expression);
		return std::to_string(-stof(factor(expression)));
	}
	else {
		std::string var = variable(expression);
		if (is_var(var)) {
			var = get_var(var);
			var.erase(0, 1);
			return var;
		} else {
			if (var == "true") {
				return "btrue";
			}
			if (var == "false") {
				return "bfalse";
			}
			raise("Unknown Variable \"" + var +  "\"", "FIXME (factor function)", "SyntaxError:");
		}
	}
	return "0"; // error
}

std::string term(const char * & expression) {
	if (debug) std::cout << "term()" << std::endl; 
	std::string result = factor(expression);
	while (peek(expression) == '*' || peek(expression) == '/') {
		if (get(expression) == '*') {
			std::string calc = factor(expression);
			char calc_type = calc[0];
			char res_type = result[0];
			calc.erase(0, 1);
			result.erase(0, 1);
			std::cout << calc << "|" << result << std::endl;
			std::cout << calc_type << "|" << res_type << std::endl;
			if (calc_type == 'i' and res_type == 'i') {
				result = "i" + std::to_string(stoi(result) * stoi(calc));
			} else if (calc_type == 'i' and res_type == 's') {
				std::string res;
				for (int i = 0; i < stoi(calc); i++) {
					res += result;
				}
				result.erase(0, 1 + stoi(calc));
				result += "s" + res;
			}
		}
		else {
			std::string calc = factor(expression);
			char calc_type = calc[0];
			char res_type = result[0];
			calc.erase(0, 1);
			result.erase(0, 1);
			if (calc_type == 'i' and res_type == 'i') {
				result = "i" + std::to_string(stof(result) / stof(calc));
			}
		}
	}
	return result;
}

std::string add_sub(const char * & expression) {
	if (debug) std::cout << "add_sub()" << std::endl; 
	std::string result = term(expression);
	while (peek(expression) == '+' || peek(expression) == '-') {
		if (get(expression) == '+') {
			if (debug) std::cout << "+" << std::endl; 
			std::string calc = term(expression);
			char calc_type = calc[0];
			char res_type = result[0];
			calc.erase(0, 1);
			result.erase(0, 1);
			std::cout << calc << "|" << result << std::endl;
			std::cout << calc_type << "|" << res_type << std::endl;
			if (calc_type == 'i' and res_type == 'i') {
				result = "i" + std::to_string(stoi(result) + stoi(calc));
			}
			else {
				//FIXME
				result = "s" + result + calc;
			}
		}
		else {
			if (debug) std::cout << "-" << std::endl; 
			std::string calc = term(expression);
			char calc_type = calc[0];
			char res_type = result[0];
			std::cout << calc << "|" << result << std::endl;
			std::cout << calc_type << "|" << res_type << std::endl;
			calc.erase(0, 1);
			result.erase(0, 1);
			if (calc_type == 'i' and res_type == 'i') {
				result = "i" + std::to_string(stoi(result) - stoi(calc));
			}
		}
	}
	return result;
}

std::string bool_cal(const char * & expression) {
	if (debug) std::cout << "bool_cal()" << std::endl; 
	std::string result = add_sub(expression);
	while (peek(expression) == '=') {
		if (get(expression) == '=') {
			if (debug) std::cout << "=" << std::endl; 
			std::string calc = add_sub(expression);
			char calc_type = calc[0];
			char res_type = result[0];
			calc.erase(0, 1);
			result.erase(0, 1);
			std::cout << calc << "|" << result << std::endl;
			std::cout << calc_type << "|" << res_type << std::endl;
			if (calc_type == 'b' and res_type == 'b') {
				if (calc == result) {
					result = "btrue";
				} else {
					result = "bfalse";
				}
			} else if (calc_type == 'i' and res_type == 'i') {
				if (stof(calc) == stof(result)) {
					result = "btrue";
				} else {
					result = "bfalse";
				}
			} else if (calc_type == 's' and res_type == 's') {
				if (calc == result) {
					result = "btrue";
				} else {
					result = "bfalse";
				}
			}
		}
	}
	return result;
}

std::string calculate(const char * & expression) {
	if (debug) std::cout << "calculate()" << std::endl; 
	std::string result = bool_cal(expression);
	return result;
}

//bool functions
std::string both(std::string bool1, std::string bool2) {
	if (solve_bool(bool1) == "true" and solve_bool(bool2) == "true") {
		return "true";
	}
	return "false";
}

std::string equals(std::string item1, std::string item2) {
	if (solve(item1) == solve(item2)) {
		return "true";
	}
	return "false";
}

std::string bool_not(std::string bool1) {
	if (solve_bool(bool1) != "true") {
		return "true";
	}
	return "false";
}

//internal functions
std::vector<std::string> split(std::string string, char delim) {
	string.erase(0, string.find_first_not_of(" \n\r\t"));
	string.erase(string.find_last_not_of(" \n\r\t")+1);
	std::stringstream ss(string);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(ss, segment, delim)) {
		segment.erase(0, segment.find_first_not_of(" \n\r\t"));
		segment.erase(segment.find_last_not_of(" \n\r\t")+1);
		seglist.push_back(segment);
	}

	return seglist;
}

bool is_int(std::string string) {
	for (int i = 0; i < string.size(); i++) {
		if (!(isdigit(string[i]))) {
			return false;
		}
	}

	return true;
}

bool is_float(std::string string) {
	for (int i = 0; i < string.size(); i++) {
		if (!(isdigit(string[i]) or string[i] == '.')) {
			return false;
		}
	}

	return true;
}

void def_func(std::string name, std::vector<std::string> args, std::string start_line, std::string end_line) {
	vars[variable_scope + "f" + name] = name;
	std::map<std::string, std::string> func;
	func.insert(std::make_pair("start", start_line));
	func.insert(std::make_pair("end", end_line));
	func.insert(std::make_pair("args", std::to_string(args.size())));
	for (int i = 0; i < args.size(); i++) {
		func.insert(std::make_pair("arg" + std::to_string(i), args[i]));
	}
	funcs.insert(std::make_pair(name, func));
}

void set_var(std::string name, std::string val) {
	if (val[0] == '"') {
		vars[variable_scope + name] = " s" + val.substr(1, val.size()-2);
	} else if (val == "true" or val == "false") {
		vars[variable_scope + name] = " b" + val;
	} else {
		vars[variable_scope + name] = " i" + val;
	}
}

std::string raise(std::string text, std::string line, std::string type) {
	std::string msg = type + " on line " + line + ": " + text;
	std::cout << msg << std::endl;
	if (type == "Warning") {
		warnings.push_back(msg);
	} else {
		errors.push_back(msg);
	}

	return msg;
}

std::string get_var(std::string name) {
	return vars[variable_scope + name];
}

bool is_var(std::string var) {
	if (vars.find(variable_scope + var) != vars.end()) {
		return true;
	}
	return false;
}

char get_type(std::string var) {
	if (var[0] == '"') {
		return 's';
	} else if (var == "true" or var == "false") {
		return 'b';
	} else if (is_int(var)) {
		return 'i';
	} else if (is_float(var)) {
		return 'i';
	}
	return 'e';
}

std::string bool_to_bstring(bool value) {
	if (value) {
		return "btrue";
	} else {
		return "bfalse";
	}
}

std::string solve_bool(std::string to_solve) {
	if (to_solve == "true") {
		return "true";
	} else {
		return "false";
	}
}

bool is_func(std::string string) {
	string.erase(0, string.find_first_not_of(" \n\r\t"));
	string.erase(string.find_last_not_of(" \n\r\t")+1);

	char c;
	for (int i = 0; i < string.size(); i++) {
		c = string[i];
		if (find(variable_chars.begin(), variable_chars.end(), c) != variable_chars.end()) {
			continue;
		} else if (c == '(') {
			return true;
		} else {
			return false;
		}
	}
}

std::vector<std::string> split_add_string(std::string string, char delim, std::string add_string) {
	string.erase(0, string.find_first_not_of(" \n\r\t"));
	string.erase(string.find_last_not_of(" \n\r\t")+1);
	std::stringstream ss(string);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(ss, segment, delim)) {
		segment.erase(0, segment.find_first_not_of(" \n\r\t"));
		segment.erase(segment.find_last_not_of(" \n\r\t")+1);
		seglist.push_back(segment + add_string);
	}

	return seglist;
}

std::string solve(std::string to_solve) {
	to_solve.erase(0, to_solve.find_first_not_of(" \n\r\t"));
	to_solve.erase(to_solve.find_last_not_of(" \n\r\t")+1);
	if (to_solve == "") {
		return "";
	}
	const char * calculate_solve = to_solve.c_str();
	if (is_func(to_solve)) {
		return line(to_solve, 0, false);
	}
	return calculate(calculate_solve).erase(0, 1);

	if (is_var(to_solve)) {
		to_solve = get_var(to_solve);
		to_solve.erase(0, 1);
		return to_solve;
	}
}

std::string run_func(std::string func, std::vector<std::vector<std::string> > args) {
	std::map<std::string, std::string> func_map;
	func_map = funcs[func];
	std::string args_string = ":" + func_map["args"];
	for (int i = 0; i < stoi(func_map["args"]); i++) {
		args_string += ":" + func_map["arg"+std::to_string(i)];
	}

	return "block:func:" + func_map["start"] + ":" + func_map["end"] + args_string;
}

std::string built_in_func(std::string func, std::vector<std::vector<std::string> > args) {
	if (func == "print") {
		std::string string;
		for (int i = 1; i < args.size(); i++) {
			string += solve(args[i][0]);
		}
		std::cout << string << std::endl;

		return string;
	}
	if (func == "input") {
		std::string string;
		for (int i = 1; i < args.size(); i++) {
			string += solve(args[i][0]);
		}
		std::cout << string;

		std::string value;
		std::getline(std::cin, value);
		return value;
	}
	if (func == "eval") {
		return solve(line(args[1][0], 0, false));
	}
	if (func == "type") {
		std::string type;
		type.push_back(get_type(args[1][0]));
		return type;
	}
	if (func == "list") {
		return "l";
	}
	if (func == "append") {
		return "l";
	}
}

std::string parse_funcs(std::vector<std::vector<std::string> > tokens_pair) {
	/*
	int lowest = 0;
	for (auto & element : tokens_pair) {
		if (stoi(element[1]) > lowest) {
			lowest = stoi(element[1]);
		}
	}
	
	std::cout << lowest << std::endl;
	
	int solved_layers = 0;
	bool solved = false;
	bool error = false;
	while (solved == false and error == false) {
		
	}
	*/

	std::vector<std::string> args;

	for (int i = 1; i < tokens_pair.size(); i++) {
		args.push_back(tokens_pair[i][0]);
	}

	//Change me!
	if (vars.find(variable_scope + "f" + tokens_pair[0][0]) != vars.end()) {
		return run_func(tokens_pair[0][0], tokens_pair);
	} else {
		return built_in_func(tokens_pair[0][0], tokens_pair);
	}
}

std::string func_lex(std::string line, bool debug_mode) {
	int par = 0;
	int last_level = 0;
	std::vector<std::string> tokens;
	std::vector<std::vector<std::string> > tokens_pair;
	std::vector<std::string> tokens_pair_template;
	std::string token = "";
	line += ' ';

	/*
	bool in_string = false;
	for (auto i = 0; i != line.size(); i++) {
		char c = line[i];
		if (in_string) {
			if (c == '"') {
				in_string = false;
			}
			token += c;
			last_level = par;
		} else {
			if (c == '"') {
				in_string = true;
			}
			if (c == ')' or c == '(') {
				token.erase(0, token.find_first_not_of(" \n\r\t"));
				token.erase(token.find_last_not_of(" \n\r\t")+1);

				tokens.push_back(token);

				tokens_pair_template.push_back(token);
				tokens_pair_template.push_back(std::to_string(par));

				tokens_pair.push_back(tokens_pair_template);
				tokens_pair_template.clear();

				//all code here for debug only:
				if (debug_mode) {
					std::cout << par;
					std::cout << token << std::endl;
				}
				//end debug

				token = "";
				last_level == par;
			}
			if (c == '(') {
				par = par + 1;
			} else if (c == ')') {
				par = par - 1;
			} else {
				token += c;
			}
			last_level = par;
		}
	}
	*/

	bool in_string = false;
	for (auto i = 0; i != line.size(); i++) {
		char c = line[i];
		if (in_string) {
			if (c == '"') {
				in_string = false;
			}
			token += c;
		} else {
			if (c == '"') {
				in_string = true;
			}
			if (c == ',' or c == ')' and par == 1 or c == '(' and par == 0) {
				token.erase(0, token.find_first_not_of(" \n\r\t"));
				token.erase(token.find_last_not_of(" \n\r\t")+1);

				tokens.push_back(token);

				tokens_pair_template.push_back(token);
				tokens_pair_template.push_back(std::to_string(par));

				tokens_pair.push_back(tokens_pair_template);
				tokens_pair_template.clear();

				//all code here for debug only:
				if (debug_mode) {
					std::cout << par << "|";
					std::cout << token << std::endl;
				}
				//end debug

				token = "";
			}
			if (c == '(') {
				if (par != 0) {
					token += c;
				}
				par += 1;
			} else if (c == ')') {
				if (par != 1) {
					token += c;
				}
				par -= 1;
			} else if (c == ',') {

			} else {
				token += c;
			}
		}
	}
	//trims white-spaces
	token.erase(0, token.find_first_not_of(" \n\r\t"));
	token.erase(token.find_last_not_of(" \n\r\t")+1);
	if (token == "do") {
		//call block-parse (parse_block(tokens))

		//all code here for debug only:
		tokens.push_back(token);

		tokens_pair_template.push_back(token);
		tokens_pair_template.push_back(std::to_string(par));

		tokens_pair.push_back(tokens_pair_template);
		tokens_pair_template.clear();

		if (debug_mode) {
			std::cout << par << "then do" << std::endl;
		}

		token = "";
		//last_level == par;
		//end debug
	} else if (token == "end") {
		//close block
		if (debug_mode) {
			std::cout << line << std::endl;
		}
	} else {
		//calls func-parse
		return parse_funcs(tokens_pair);
	}

	return "nil";
}

std::string var_lex(std::string line, bool debug_mode) {
	std::stringstream str(line);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(str, segment, ':')) {
		segment.erase(0, segment.find_first_not_of(" \n\r\t"));
		segment.erase(segment.find_last_not_of(" \n\r\t")+1);
		seglist.push_back(segment);
	}
	seglist[1] = line.substr(line.find(":") + 1);
	seglist[1].erase(0, seglist[1].find_first_not_of(" \n\r\t"));
	seglist[1].erase(seglist[1].find_last_not_of(" \n\r\t")+1);
	if (debug_mode) {
		std::cout << seglist[0] << ": " << seglist[1] << std::endl;
	}

	set_var(seglist[0], solve(seglist[1]));
	return get_var(seglist[0]);
}

std::string block_lex(std::vector<std::string> seglist, int line_num, bool debug_mode) {
	if (seglist.size() < 2) {
		return raise("Error: to few args", "0", "Syntax");
	}

	if (debug_mode) {
		std::cout << seglist[0] << " " << seglist[1] << " block" << std::endl;
	}

	if (seglist[0] == "if") {
		std::string if_bool = solve_bool(seglist[1]);

		if (if_bool == "true") {
			return "block:do";
		} else {
			return "block:skip";
		}
	}

	if (seglist[0] == "while") {
		return "block:while:" + seglist[1];
	}

	if (seglist[0] == "def") {
		return "block:def:" + seglist[1] + ":" + std::to_string(line_num);
	}

	return "nil";
}

std::vector<std::string> pre_lex(std::string line) {
	line.erase(0, line.find_first_not_of(" \n\r\t"));
	line.erase(line.find_last_not_of(" \n\r\t")+1);
	std::stringstream str(line);
	std::string segment;
	std::vector<std::string> seglist;

	while (std::getline(str, segment, ' ')) {
		segment.erase(0, segment.find_first_not_of(" \n\r\t"));
		segment.erase(segment.find_last_not_of(" \n\r\t")+1);
		seglist.push_back(segment);
	}

	return seglist;
}

std::string line(std::string line, int line_num, bool debug_mode) {
	std::vector<std::string> tokens = pre_lex(line);

	if (tokens[tokens.size()-1] == "do") {
		return block_lex(tokens, line_num, debug_mode);
	} else {
		bool in_string = false;
		std::string lex = "func";
		for (auto c : line) {
			if (in_string) {
				if (c == '"') {
					in_string = false;
				}
			} else {
				if (c == '"') {
					in_string = true;
				} else if (c == ':') {
					lex = "var";
				}
			}
		}
		if (lex == "func") {
			return func_lex(line, debug_mode);
		} else {
			return var_lex(line, debug_mode);
		}
	}

	return "nil";
}

void run(std::string file, bool debug_mode) {
	std::stringstream filestream(file);
	std::string segment;
	std::vector<std::string> lines;

	while (std::getline(filestream, segment, '\n')) {
		segment.erase(0, segment.find_first_not_of(" \n\r\t"));
		segment.erase(segment.find_last_not_of(" \n\r\t")+1);
		lines.push_back(segment);
	}

	int line_num = 0;
	std::string status;
	std::vector<std::string> block_data_temp;
	std::vector<std::vector<std::string>> block_data;
	int block = 0;
	while (true) {
		if (block > 0) {
			if (lines[line_num] == "end") {
				if (split(block_data.back()[0], ':')[1] == "skip") {
					block -= 1;
					block_data.pop_back();
					line_num++;
					continue;
				}
				if (split(block_data.back()[0], ':')[1] == "do") {
					block -= 1;
					block_data.pop_back();
					line_num++;
					continue;
				}
				if (split(block_data.back()[0], ':')[1] == "while") {
					std::string if_bool = solve_bool(split(block_data.back()[0], ':')[2]);

					if (if_bool == "true") {
						line_num = stoi(block_data.back()[1]);
					} else {
						block -= 1;
						block_data.pop_back();
						line_num++;
						continue;
					}
				}
				if (split(block_data.back()[0], ':')[1] == "def") {
					def_func(split(block_data[0][0], ':')[2], {}, split(block_data[0][0], ':')[3], std::to_string(line_num));
					block -= 1;
					block_data.pop_back();
					line_num++;
					continue;
				}
				if (split(block_data.back()[0], ':')[1] == "func") {
					line_num = stoi(block_data.back()[1]) + 1;
					block -= 1;
					block_data.pop_back();
					variable_scope = "";
					continue;
				}
			}
			if (split(lines[line_num], ' ')[0] == "return") {

			}
			if (split(block_data.back()[0], ':')[1] == "skip") {
				line_num++;
				continue;
			}
			if (split(block_data.back()[0], ':')[1] == "do") {
			}
			if (split(block_data.back()[0], ':')[1] == "while") {
			}
			if (split(block_data.back()[0], ':')[1] == "def") {
				line_num++;
				continue;
			}
			if (split(block_data.back()[0], ':')[1] == "func") {
			}
		}

		if (lines.size() < line_num + 1) {
			if (debug_mode) {
				std::cout << "exiting" << std::endl;
			}
			break;
		}

		status = line(lines[line_num], line_num, debug_mode);

		if (debug_mode) {
			std::cout << status << std::endl;
		}

		if (status.find(':') == status.npos) {

		} else if (split(status, ':')[0] == "block") {
			block++;
			block_data_temp.clear();
			block_data_temp.push_back(status);
			block_data_temp.push_back(std::to_string(line_num));
			block_data.push_back(block_data_temp);
			block_data_temp.clear();

			if (split(block_data.back()[0], ':')[1] == "func") {
				variable_scope = "~";
				line_num = stoi(split(block_data.back()[0], ':')[2]);
			}
		}

		line_num++;
	}
}

int main() {
	//test
	run(
	"def test do\n"
		"print(\"It Worked!\")\n"
		"x: 120\n"
		"print(x)\n"
		"hi()\n"
	"end\n"
	"def hi do\n"
		"print(\"Hello!\")\n"
		"print(x)\n"
	"end\n"

	"x: 110\n"
	"test()\n"
	"print(x)", false);
	//end test
}

/*
x = val
 ^ ^
...function(args)...

if (true) do
  ^      ^
end

while (true) do
     ^      ^
end

for (list, args) do
   ^            ^
end

x: "string 1"
y: "string 2"
if true do
	print(x)
end
if false do
	print(x)
end

x: "string 1"
y: "string 2"
if true do
	print(x+" and "+y)
	z: input("testing: ")
	print(z)
	print(print(input("testing: "))," test")
	print(input("testing: "),input("testing2: "))
	print("This will print")
	print(input("type input: "))
end
if false do
	print("This will not print")
end
*/