/*
 * JsonParse.h
 *
 *  Created on: 2015-7-14
 *      Author: qianqians
 */
#ifndef _JsonParse_h
#define _JsonParse_h

#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <memory>

#include <boost/unordered_map.hpp>
#include <boost/any.hpp>

#include <allocator.h>

namespace Fossilizid{
namespace JsonParse{

class jsonformatexception : public std::exception{
public:
	jsonformatexception(char * str) : std::exception(str){
	}

};

typedef std::nullptr_t JsonNull;
typedef bool JsonBool;
typedef std::string JsonString;
typedef std::int64_t JsonInt;
typedef std::double_t JsonFloat;
typedef boost::any JsonObject;
typedef std::shared_ptr<boost::unordered_map<std::string, JsonObject, boost::hash<std::string>, std::equal_to<std::string>, allocator<std::pair<std::string, JsonObject> > > > JsonTable;
inline JsonTable Make_JsonTable(){
	return std::make_shared<boost::unordered_map<std::string, JsonObject, boost::hash<std::string>, std::equal_to<std::string>, allocator<std::pair<std::string, JsonObject> > > >();
}
typedef std::shared_ptr<std::vector<JsonObject, allocator<JsonObject> > > JsonArray;
inline JsonArray Make_JsonArray(){
	return std::make_shared<std::vector<JsonObject, allocator<JsonObject> > >();
}
static JsonNull JsonNull_t = nullptr;

template <class T>
inline T JsonCast(JsonObject & o){
	return boost::any_cast<T>(o);
}

inline void _pre_process(JsonString v, std::string & _out){
	for (auto c : v){
		if (c == '\"'){
			_out += "\\\"";
		}
		else if (c == '\\') {
			_out += "\\\\";
		}
		else{
			_out += c;
		}
	}
}

inline void _pack(JsonString v, std::string & _out){
	return _pre_process(v, _out);
}

inline void _pack(JsonInt v, std::string & _out){
	_out += std::to_string(v);
}

inline void _pack(JsonFloat v, std::string & _out){
	_out += std::to_string(v);
}

inline void _pack(JsonBool v, std::string & _out){
	if (boost::any_cast<bool>(v)){
		_out += "true";
	}
	else {
		_out += "false";
	}
}

inline void _pack(JsonNull v, std::string & _out){
	_out += "null";
}

inline void pack(JsonTable & o, std::string & _out);

inline void pack(JsonArray & _array, std::string & _out);

inline void pack(JsonObject & v, std::string & _out){
	if (v.type() == typeid(std::string) || v.type() == typeid(const char *) || v.type() == typeid(char const*) || v.type() == typeid(char*)){
		_out += "\"";
	}

	if (v.type() == typeid(const char *)) {
		_pack(std::string(boost::any_cast<const char *>(v)), _out);
	} else if (v.type() == typeid(char const*)) {
		_pack(std::string(boost::any_cast<char const*>(v)), _out);
	} else if (v.type() == typeid(char*)) {
		_pack(std::string(boost::any_cast<char*>(v)), _out);
	} else if (v.type() == typeid(std::string)){
		_pack(boost::any_cast<JsonString>(v), _out);
	} else if (v.type() == typeid(bool)){
		_pack(boost::any_cast<JsonBool>(v), _out);
	} else if (v.type() == typeid(std::int64_t)){
		_pack((JsonInt)boost::any_cast<std::int64_t>(v), _out);
	} else if (v.type() == typeid(std::int32_t)){
		_pack((JsonInt)boost::any_cast<std::int32_t>(v), _out);
	} else if (v.type() == typeid(std::uint64_t)){
		_pack((JsonInt)boost::any_cast<std::uint64_t>(v), _out);
	} else if (v.type() == typeid(std::uint32_t)){
		_pack((JsonInt)boost::any_cast<std::uint32_t>(v), _out);
	} else if (v.type() == typeid(double)){
		_pack(boost::any_cast<JsonFloat>(v), _out);
	} else if (v.type() == typeid(std::nullptr_t)){
		_pack(nullptr, _out);
	} else if (v.type() == typeid(JsonTable) || v.type() == typeid(std::shared_ptr<boost::unordered_map<std::string, boost::any> >)){
		pack(boost::any_cast<JsonTable>(v), _out);
	} else if (v.type() == typeid(JsonArray) || v.type() == typeid(std::shared_ptr<std::vector<boost::any> >)){
		pack(boost::any_cast<JsonArray>(v), _out);
	}

	if (v.type() == typeid(std::string) || v.type() == typeid(const char *) || v.type() == typeid(char const*) || v.type() == typeid(char*)){
		_out += "\"";
	}
}

inline void pack(JsonArray & _array, std::string & _out){
	_out += "[";
	for (auto o : *_array){
			pack(o, _out);
		_out += ",";
	}
	if (_array->size() > 0) {
		_out.erase(_out.length() - 1);
	}
	_out += "]";
}

inline void pack(JsonTable & o, std::string & _out){
	_out += "{";
	for(auto _obj : *o){
		_out += "\"";
		_pack(_obj.first, _out);
		_out += "\":";
		pack(_obj.second, _out);
		_out += ",";
	}
	if (o->size() > 0) {
		_out.erase(_out.length() - 1);
	}
	_out += "}";
}

inline std::string packer(JsonObject & o){
	std::string _out;
	pack(o, _out);

	return _out;
}

inline int unpack(JsonObject & out, JsonString s){
	int begin = 0;
	auto len = s.length();
	while (s[begin] != '[' && s[begin] != '{'){
		begin++;

		if (begin >= len){
			return 0;
		}
	}

	size_t end = len;
	const char * c = s.c_str() + begin;
	int type = 0;
	JsonObject obj;
	std::list<JsonObject> pre;
	JsonTable obj_table = nullptr;
	JsonArray obj_array = nullptr;
	len = end - begin + 1; 
	int i = 0;
	std::string key;
	std::string v;

	for (; i < len; ++i){
		if (c[i] == '{'){
			obj = obj_table = Make_JsonTable();
			pre.push_back(obj);
		parsemap:
			int begin = -1, end = -1;
			
			if (c[i] == '}') {
				goto mapend;
			}

			while (1){
				begin = ++i;
				if (c[begin] == ' ' || c[begin] == '\0' || c[begin] == '\t' || c[begin] == '\n'){
					continue;
				} else if (c[i] == '}') {
					goto mapend;
				} else{
					++i;
					break;
				}
			}

			if (c[begin] != '\"'){
				throw jsonformatexception("error json fromat: not a conform key");
			}

			key.clear();
			for (; i < len; ++i) {
				if (c[i] == '\\') {
					key.push_back(c[++i]);
					continue;
				}

				if (c[i] == '\"') {
					end = i++;
					break;
				}
				key.push_back(c[i]);
			}
			if (end == -1){
				throw jsonformatexception("error json fromat: not a conform key");
			}

			while (1){
				if (c[i] == ':' || c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
					++i;
				} else{
					break;
				}
			}

			type = 1;
			goto parse;

		mapend:
			obj = pre.back();
			pre.pop_back();

			if (pre.empty()){
				break;
			}

			++i;

			if (obj.type() == typeid(JsonTable)){
				obj_table = boost::any_cast<JsonTable>(obj);
				goto parsemap;
			} else if (obj.type() == typeid(JsonArray)){
				obj_array = boost::any_cast<JsonArray>(obj);
				goto parselist;
			} else{
				continue;
			}

		} else if (c[i] == '['){
			obj = obj_array = Make_JsonArray();
			pre.push_back(obj);
		parselist:
			if (c[i] == ']') {
				goto listend;
			}

			while (1){
				++i;
				if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
					continue;
				} else if (c[i] == ']') {
					goto listend;
				} else {
					break;
				}
			}

			{
				type = 2;
				goto parse;
			}

		listend:
			obj = pre.back();
			pre.pop_back();

			if (pre.empty()){
				break;
			}
			
			++i;

			if (obj.type() == typeid(JsonTable)){
				obj_table = boost::any_cast<JsonTable>(obj);
				goto parsemap;
			} else if (obj.type() == typeid(JsonArray)){
				obj_array = boost::any_cast<JsonArray>(obj);
				goto parselist;
			} else{
				continue;
			}
		}

	parse:
		if (type == 1){
			int vbegin = i, vend = 0, count = 0;
			if (c[vbegin] == '\"'){
				count = 1;
				++i;

				v.clear();
				for (; i < len; ++i){
					if (c[i] == '\\'){
						v.push_back(c[++i]);
						continue;
					}

					if (c[i] == '\"'){
						break;
					}

					v.push_back(c[i]);
				}

				if (c[i] != '\"'){
					throw jsonformatexception("error json fromat error");
				}else{
					if (count != 1){
						throw jsonformatexception("error json fromat: can not be resolved value");
					}

					count = 0;
					vend = i++;

					while (1){
						if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
							++i;
							continue;
						} else{
							if ((c[i] == ',') || (c[i] == '}')){
								break;
							} else{
								throw jsonformatexception("error json fromat: can not be resolved value");
							}
						}
					}
				}
			
				if ((c[i] == ',') || (c[i] == '}')){
					obj_table->insert(std::make_pair(key, v));
				}

				if (c[i] == '}') {
					goto mapend;
				} else if (c[i] == ']'){
					throw jsonformatexception("error json fromat: not a array");
				} 
			} else if ((c[i]) == 'n'){
				if (c[++i] != 'u'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'l'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'l'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				++i;

				while (1){
					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						++i;
						continue;
					} else{
						if (c[i] == ',' || c[i] == '}'){
							break;
						} else{
							throw jsonformatexception("error json fromat: can not be resolved value");
						}
					}
				}

				if ((c[i] == ',') || (c[i] == '}')){
					obj_table->insert(std::make_pair(key, JsonNull_t));
				}

				if (c[i] == '}') {
					goto mapend;
				} else if (c[i] == ']'){
					throw jsonformatexception("error json fromat: not a array");
				}
			} else if ((c[i]) == 't'){
				if (c[++i] != 'r'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'u'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'e'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				++i;

				while (1){
					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						++i;
						continue;
					} else{
						if (c[i] == ',' || c[i] == '}'){
							break;
						} else{
							throw jsonformatexception("error json fromat: can not be resolved value");
						}
					}
				}

				if ((c[i] == ',') || (c[i] == '}')){
					obj_table->insert(std::make_pair(key, true));
				}

				if (c[i] == '}') {
					goto mapend;
				} else if (c[i] == ']'){
					throw jsonformatexception("error json fromat: not a array");
				}
			} else if ((c[i]) == 'f'){
				if (c[++i] != 'a'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'l'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 's'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'e'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				++i;

				while (1){
					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						++i;
						continue;
					} else{
						if (c[i] == ',' || c[i] == '}'){
							break;
						} else{
							throw jsonformatexception("error json fromat: can not be resolved value");
						}
					}
				}

				if ((c[i] == ',') || (c[i] == '}')){
					obj_table->insert(std::make_pair(key, false));
				}

				if (c[i] == '}') {
					goto mapend;
				} else if (c[i] == ']'){
					throw jsonformatexception("error json fromat: not a array");
				}
			} else if ((c[i]) == '['){
				auto _new = Make_JsonArray();
				obj_table->insert(std::make_pair(key, _new));
				pre.push_back(obj);
				obj = obj_array = _new;
				goto parselist;
			} else if ((c[i]) == '{'){
				auto _new = Make_JsonTable();
				obj_table->insert(std::make_pair(key, _new));
				pre.push_back(obj);
				obj = obj_table = _new;
				goto parsemap;
			} else {
				bool isint = true;
				while (1){
					if ((c[i++] > '9' && c[i] < '0') && c[i] != '.' && c[i] != ' ' && c[i] != '\0'){
						throw jsonformatexception("error json fromat: can not be resolved value");
					}

					if (c[i] == '.'){
						isint = false;
						count++;
					}

					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						vend = i;
						while (1){
							if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
								++i;
								continue;
							} else{
								if (c[i] == ',' || c[i] == '}'){
									break;
								}

								throw jsonformatexception("error json fromat: can not be resolved value");
							}
						}
					}

					if (c[i] == ',' || c[i] == '}'){
						vend = i;
						break;
					}
				}
			
				std::string str(&c[vbegin], vend - vbegin);
				if ((c[i] == ',') || (c[i] == '}')){
					if (isint){
						obj_table->insert(std::make_pair(key, atoll(str.c_str())));
					} else{
						obj_table->insert(std::make_pair(key, atof(str.c_str())));
					}
				}

				if (c[i] == '}') {
					goto mapend;
				} else if (c[i] == ']'){
					throw jsonformatexception("error json fromat: not a array");
				}
			}

			goto parsemap;

		}else if (type == 2){
			int vbegin = i, vend = 0, count = 0;
			if (c[vbegin] == '\"'){
				count = 1;
				++i;

				v.clear();
				for (; i < len; ++i){
					if (c[i] == '\\'){
						v.push_back(c[++i]);
						continue;
					}

					if (c[i] == '\"'){
						break;
					}

					v.push_back(c[i]);
				}

				if (c[i] != '\"'){
					throw jsonformatexception("error json fromat error");
				} else{
					if (count != 1){
						throw jsonformatexception("error json fromat: can not be resolved value");
					}

					count = 0;
					vend = i++;

					while (1){
						if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
							++i;
							continue;
						} else{
							if ((c[i] == ',') || (c[i] == ']')){
								break;
							} else{
								throw jsonformatexception("error json fromat: can not be resolved value");
							}
						}
					}
				}

				if ((c[i] == ',') || (c[i] == ']')){
					obj_array->push_back(v);
				}

				if (c[i] == '}') {
					throw jsonformatexception("error json fromat: not a dict");
				} else if (c[i] == ']'){
					goto listend;
				}
			} else if ((c[i]) == 'n'){
				if (c[++i] != 'u'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'l'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'l'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				++i;

				while (1){
					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						++i;
						continue;
					} else{
						if (c[i] == ',' || c[i] == ']'){
							break;
						} else{
							throw jsonformatexception("error json fromat: can not be resolved value");
						}
					}
				}

				if ((c[i] == ',') || (c[i] == ']')){
					obj_array->push_back(JsonNull_t);
				}

				if (c[i] == '}') {
					throw jsonformatexception("error json fromat: not a dict");
				} else if (c[i] == ']'){
					goto listend;
				}
			} else if ((c[i]) == 't'){
				if (c[++i] != 'r'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'u'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'e'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				++i;

				while (1){
					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						++i;
						continue;
					} else{
						if (c[i] == ',' || c[i] == ']'){
							break;
						} else{
							throw jsonformatexception("error json fromat: can not be resolved value");
						}
					}
				}

				if ((c[i] == ',') || (c[i] == ']')){
					obj_array->push_back(true);
				}

				if (c[i] == ']') {
					goto listend;
				}
				else if (c[i] == '}') {
					throw jsonformatexception("error json fromat: not a dict");
				} 
			} else if ((c[i]) == 'f'){
				if (c[++i] != 'a'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'l'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 's'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				if (c[++i] != 'e'){
					throw jsonformatexception("error json fromat: can not be resolved value");
				}
				++i;

				while (1){
					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						++i;
						continue;
					} else{
						if (c[i] == ',' || c[i] == ']'){
							break;
						} else{
							throw jsonformatexception("error json fromat: can not be resolved value");
						}
					}
				}

				if ((c[i] == ',') || (c[i] == ']')){
					obj_array->push_back(false);
				}

				if (c[i] == '}') {
					throw jsonformatexception("error json fromat: not a dict");
				} else if (c[i] == ']'){
					goto listend;
				}
			} else if ((c[i]) == '[') {
				auto _new = Make_JsonArray();
				obj_array->push_back(_new);
				pre.push_back(obj);
				obj = obj_array = _new;
				goto parselist;
			} else if ((c[i]) == '{') {
				auto _new = Make_JsonTable();
				obj_array->push_back(_new);
				pre.push_back(obj);
				obj = obj_table = _new;
				goto parsemap;
			} else {
				bool isint = true;
				while (1){
					if ((c[i++] > '9' && c[i] < '0') && c[i] != '.' && c[i] != ' ' && c[i] != '\0'){
						throw jsonformatexception("error json fromat: can not be resolved value");
					}

					if (c[i] == '.'){
						isint = false;
						count++;
					}

					if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
						vend = i;
						while (1){
							if (c[i] == ' ' || c[i] == '\0' || c[i] == '\t' || c[i] == '\n'){
								++i;
								continue;
							} else{
								if (c[i] == ',' || c[i] == ']'){
									break;
								}

								throw jsonformatexception("error json fromat: can not be resolved value");
							}
						}
					}

					if (c[i] == ',' || c[i] == ']'){
						vend = i;
						break;
					}
				}

				std::string str(&c[vbegin], vend - vbegin);
				if ((c[i] == ',') || (c[i] == ']')){
					if (isint){
						obj_array->push_back(atoll(str.c_str()));
					} else{
						obj_array->push_back(atof(str.c_str()));
					}
				}

				if (c[i] == '}') {
					throw jsonformatexception("error json fromat: not a dict");
				} else if (c[i] == ']'){
					goto listend;
				}
			}

			goto parselist;

		}
	}
	
	out = obj;

	return i;
}

inline int unpacker(JsonObject & out, JsonString s){
	return unpack(out, s);
}

} /* namespace JsonParse */
} /* namespace Fossilizid */

#endif //_routing_h
