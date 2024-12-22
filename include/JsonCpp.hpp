#ifndef __LIBRALFOGIT_JSONCPPWRAPPER_HPP__
#define __LIBRALFOGIT_JSONCPPWRAPPER_HPP__

/*
 * Copyright(C) 2022 RalfO. All rights reserved.
 * https://github.com/RalfOGit/libtasmota
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <Json.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#else
namespace libralfogit {
#endif

    /**
     * Class implementing a C++ wrapper for the very low-print json parsing library written by James McLaughlin
     * (see https://github.com/json-parser/json-parser).
     * It implements 
     */
    class JsonCpp {

    public:
        class JsonValue;
        class JsonNamedValue;

        /** Class encapsulating a json object value. */
        class JsonObject {
        protected:
            json_object_entry* value;   ///< pointer to an array of json_object_entry elements
            unsigned int       length;  ///< number of json_object_entry elements in the array
        public:
            JsonObject(const json_value* const jvalue = NULL) : value(NULL), length(0) {                            /// Constructor. @param pointer to the json_object_entry in the json tree
                if (jvalue != NULL && jvalue->type == json_object) {
                    value  = jvalue->u.object.values;
                    length = jvalue->u.object.length;
                }
            }
            JsonObject(const json_object_entry* const entry) : JsonObject((entry != NULL ? entry->value : NULL)) {} /// Constructor. @param pointer to the json_object_entry in the json tree
            const json_object_entry* const c_ptr   (void) const { return value; }                                   ///< Pointer to child elements in this json object. 
            const unsigned int             c_length(void) const { return length; }                                  ///< Number of child elements in this json object.

            size_t               size      (void)                   const { return length; }                        ///< Number of key value pairs in  this json object.
            const JsonNamedValue operator[](size_t index)           const { return JsonNamedValue(&value[index]); } ///< Array index operator [] for this json object.
            const JsonValue      operator[](const std::string& key) const {                                         ///< Array dictionary operator [] for this json object.
                return getValue(value, length, key, NULL);
            }
            operator std::string() const {                                                                          ///< String representation for this json object.
                std::string result = "{";
                for (auto value : *this) {
                    result.append(result.size() > 1 ? "," : "").append(std::string(value));
                }
                return result.append("}");
            }

            class iterator {
            public:
                iterator(json_object_entry* _ptr) : ptr(_ptr) {}
                iterator operator++() { ++ptr; return *this; }
                bool operator!=(const iterator& other) const { return ptr != other.ptr; }
                const JsonNamedValue operator*() const { return JsonNamedValue(ptr); }
            private:
                json_object_entry* ptr;
            };
            iterator begin() const { return iterator(value); }
            iterator end() const { return iterator(value + length); }
        };

        /** Class encapsulating a json array value. */
        class JsonArray {
        protected:
            json_value** value;     ///< pointer to an array of json_value elements
            unsigned int length;    ///< number of json_value elements in the array
        public:
            JsonArray(const json_value* const jvalue = NULL) : value(NULL), length(0) {                 /// Constructor. @param pointer to the json_object_entry in the json tree
                if (jvalue != NULL && jvalue->type == json_array) {
                    value  = jvalue->u.array.values;
                    length = jvalue->u.array.length;
                }
            }
            JsonArray(const json_object_entry* const entry = NULL) : JsonArray((entry != NULL ? entry->value : NULL)) {} /// Constructor. @param pointer to the json_object_entry in the json tree
            const json_value** const c_ptr   (void) const { return (const json_value** const)value; }   ///< Pointer to values in this json array.
            const unsigned int       c_length(void) const { return length; }                            ///< Get number of child elements for this json object.

            size_t            size(void)               const { return length; }                         ///< Number of values in this json array.
            const JsonValue   operator[](size_t index) const { return JsonValue((value[index])); }      ///< Array index operator [] for this json array.
            operator std::string() const {                                                              ///< String representation for this json array.
                std::string result = "[";
                for (auto value : *this) {
                    result.append(result.size() > 1 ? "," : "").append(std::string(value));
                }
                return result.append("]");
            }

            class iterator {
            public:
                iterator(json_value** _ptr) : ptr(_ptr) {}
                iterator operator++() { ++ptr; return *this; }
                bool operator!=(const iterator& other) const { return ptr != other.ptr; }
                const JsonValue operator*() const { return JsonValue(*ptr); }
            private:
                json_value** ptr;
            };
            iterator begin() const { return iterator(value); }
            iterator end() const { return iterator(value + length); }
        };

        /** Class encapsulating a json string value. */
        class JsonString {
        protected:
            std::string value;                                                      ///< json string value converted from the json_object_entry
        public:
            JsonString(const json_value* const jvalue = NULL) : value("INVALID") {  /// Constructor. @param pointer to the json_value in the json tree
                if (jvalue != NULL && jvalue->type == json_string) {
                    value = std::string(jvalue->u.string.ptr, jvalue->u.string.length);
                }
            }
            JsonString(const json_object_entry* const entry) : JsonString((entry != NULL ? entry->value : NULL)) {}
            const    std::string& getValue(void) const { return value; }            ///< Get string value for this json string.
            operator std::string          (void) const { return getValue(); }       ///< Get string value for this json string. Same as getValue().
        };

        /** Class encapsulating a json integer value. */
        class JsonInt {
        protected:
            long long value;                                                        ///< json integer value copied from the json_object_entry
        public:
            JsonInt(const json_value* const jvalue = NULL) : value(-99999999) {     /// Constructor. @param pointer to the json_value in the json tree
                if (jvalue != NULL && jvalue->type == json_integer) {
                    value = jvalue->u.integer;
                }
            }
            JsonInt(const json_object_entry* const entry) : JsonInt((entry != NULL ? entry->value : NULL)) {}    /// Constructor. @param pointer to the json_object_entry in the json tree
            long long getValue   (void) const { return value; }                     ///< Get value of this json integer.
            operator  std::string(void) const {                                     ///< Get string representation for this json integer.
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%lld", value);
                return buffer;
            }
        };

        /** Class encapsulating a json double value. */
        class JsonDouble {
        protected:
            double value;                                                           ///< json double value copied from the json_object_entry
        public:
            JsonDouble(const json_value* const jvalue = NULL) : value(-99999999) {  /// Constructor. @param pointer to the json_object_entry in the json tree
                if (jvalue != NULL && jvalue->type == json_double) {
                    value = jvalue->u.dbl;
                }
            }
            JsonDouble(const json_object_entry* const entry) : JsonDouble((entry != NULL ? entry->value : NULL)) {} /// Constructor. @param pointer to the json_object_entry in the json tree
            double   getValue   (void) const { return value; }                      ///< Get value of this json double.
            operator std::string(void) const {                                      ///< Get string representation for this json double.
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%lf", value);
                return buffer;
            }
        };

        /** Class encapsulating a json boolean value. */
        class JsonBool {
        protected:
            bool value;
        public:
            JsonBool(const json_value* const jvalue = NULL) : value(false) {        /// Constructor. @param pointer to the json_object_entry in the json tree
                if (jvalue != NULL && jvalue->type == json_boolean) {
                    value = jvalue->u.boolean;
                }
            }
            JsonBool(const json_object_entry* const entry) : JsonBool((entry != NULL ? entry->value : NULL)) {} /// Constructor. @param pointer to the json_object_entry in the json tree
            bool     getValue   (void) const { return value; }                      ///< Get value of this json boolean.
            operator std::string(void) const {                                      ///< Get string representation for this json boolean.
                return (value == false ? "false" : "true");
            }
        };

        /**
        * Class encapsulating a json value.
        * It holds instances of all possible json value types; only one of them is valid at any given time.
        */
        class JsonValue {
        protected:
            JsonObject  value_object;
            JsonArray   value_array;
            JsonString  value_string;
            JsonBool    value_boolean;
            JsonInt     value_int;
            JsonDouble  value_double;
            json_type   type;
        public:
            JsonValue(const json_value* const jvalue = NULL) :
                value_object(jvalue),
                value_array(jvalue),
                value_string(jvalue),
                value_boolean(jvalue),
                value_int(jvalue),
                value_double(jvalue),
                type(jvalue != NULL ? jvalue->type : json_none) {}
            JsonValue(const json_object_entry* const jvalue = NULL) :
                value_object(jvalue),
                value_array(jvalue),
                value_string(jvalue),
                value_boolean(jvalue),
                value_int(jvalue),
                value_double(jvalue),
                type(jvalue != NULL ? json_object : json_none) {}

            const json_type    getType (void) const { return type; }           ///< Get type of this json value.

            const JsonObject&  asObject(void) const { return value_object; }   ///< Get object value of this json value.
            const JsonArray&   asArray (void) const { return value_array; }    ///< Get array value of this json value.
            const JsonString&  asString(void) const { return value_string; }   ///< Get string value of this json value.
            const JsonBool&    asBool  (void) const { return value_boolean; }  ///< Get boolean value of this json value.
            const JsonInt&     asInt   (void) const { return value_int; }      ///< Get integer value of this json value.
            const JsonDouble&  asDouble(void) const { return value_double; }   ///< Get double value of this json value.

            const bool isNull  (void) const { return type == json_null; }
            const bool isNone  (void) const { return type == json_none; }
            const bool isObject(void) const { return type == json_object; }
            const bool isArray (void) const { return type == json_array; }
            const bool isString(void) const { return type == json_string; }
            const bool isBool  (void) const { return type == json_boolean; }
            const bool isInt   (void) const { return type == json_integer; }
            const bool isDouble(void) const { return type == json_double; }

            /**
            * Get the value of this json name value pair converted to a string.
            * @return the value as string
            */
            operator std::string() const { 
                switch (type) {
                case json_object:  return std::string(value_object);
                case json_string:  return std::string(value_string);
                case json_boolean: return std::string(value_boolean);
                case json_integer: return std::string(value_int);
                case json_double:  return std::string(value_double);
                case json_array:   return std::string(value_array);
                case json_null:    return "null";
                }
                return "INVALID";
            }
        };

        /**
        * Class encapsulating a json name value pair.
        * It holds instances of all possible json value types; only one of them is valid at any given time.
        */
        class JsonNamedValue : public JsonValue {
        protected:
            std::string name;
        public:
            JsonNamedValue(const json_object_entry* const entry = NULL) :
                JsonValue(entry != NULL ? entry->value : NULL) {
                if (entry != NULL && entry->name != NULL) {
                    name = std::string(entry->name, entry->name_length);
                }
                else {
                    name = "INVALID";
                }
            }
            const    std::string getName (void) const { return name; }
            const    JsonValue   getValue(void) const { return JsonValue(*this); }
            operator std::string         (void) const { return getName() + ":" + JsonValue::operator std::string(); }
        };

        /// Type definition for a vector of json named value pairs
        typedef std::vector<JsonNamedValue> JsonNamedValueVector;

        /**
        * Get a vector of json named value pairs from the given json tree level.
        * @param json  pointer to a subtree in the json tree
        * @return a vector of json named value pairs, or an empty vector
        */
        static JsonNamedValueVector getNamedValues(const json_value* const json) {
            if (json != NULL && json->type == json_object) {
                JsonObject object(json);
                return getNamedValues(object);
            }
            return JsonNamedValueVector();
        }

        /**
        * Get a vector of json named value pairs from the given json object.
        * @param elements  pointer to an object in the json tree
        * @param num_elements number of elements in the json object
        * @return a vector of json named value pairs, or an empty vector
        */
        static JsonNamedValueVector getNamedValues(const json_object_entry* const elements, const unsigned int num_elements) {
            JsonNamedValueVector named_variants;
            if (elements != NULL) {
                named_variants.reserve(num_elements);
                for (unsigned int i = 0; i < num_elements; ++i) {
                    named_variants.push_back(JsonNamedValue(&elements[i]));
                }
            }
            return named_variants;
        }

        /**
        * Get a vector of json named value pairs from the given json object.
        * @param object json object instance
        * @return a vector of json named value pairs, or an empty vector
        */
        static JsonNamedValueVector getNamedValues(const JsonObject& object) {
            JsonNamedValueVector named_variants;
            named_variants.reserve(object.size());
            for (const auto& element : object) {
                named_variants.push_back(element);
            }
            return named_variants;
        }


        typedef bool (*compare)(const std::string& lhs, const std::string& rhs);

        /**
        * Get a json named value from the given json tree level.
        * @param elements  pointer to an array of json object entries
        * @param num_elements number of array elements
        * @param name the name of the json named value pair to search for
        * @param name_comparator a function pointer to an optional name comparater method, or NULL
        * @return a json named value pair, or an empty named value pair
        */
        static JsonNamedValue getValue(const json_object_entry* const elements, const size_t num_elements, const std::string& name, compare name_comparator = NULL) {
            if (elements != NULL) {
                for (size_t i = 0; i < num_elements; ++i) {
                    std::string element_name(elements[i].name, elements[i].name_length);
                    if (name_comparator == NULL && element_name == name ||
                        name_comparator != NULL && name_comparator(element_name, name) == true) {
                        return JsonNamedValue(&elements[i]);
                    }
                }
            }
            return JsonNamedValue(NULL);
        }

        /**
        * Get a json named value from the given json tree level.
        * @param object  pointer to a json object entries
        * @param name the name of the json named value pair to search for
        * @param name_comparator a function pointer to an optional name comparater method, or NULL
        * @return a json named value pair, or an empty named value pair
        */
        static JsonNamedValue getValue(const JsonObject& object, const std::string& name, compare name_comparator = NULL) {
            for (const auto element : object) {
                std::string element_name(element.getName());
                if (name_comparator == NULL && element_name == name ||
                    name_comparator != NULL && name_comparator(element_name, name) == true) {
                    return element;
                }
            }
            return JsonNamedValue(NULL);
        }
    };

}   // namespace libphoscon

#endif
