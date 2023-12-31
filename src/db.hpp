#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <vector>
#include <thread>
#include <ctime>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <map>
#include <format>

int	ft_atoi(const char* a)
{
    int	ret;
    int	sign;

    ret = 0;
    sign = 1;
    while ((*a >= 9 && *a <= 13) || *a == 32)
    {
        a++;
    }
    if (*a == '+' || *a == '-')
    {
        if (*a == '-')
            sign = -1;
        a++;
    }
    while (*a >= '0' && *a <= '9')
    {
        ret = ret * 10 + (*a - '0');
        a++;
    }
    return (sign * ret);
}

bool isNumber(std::string a)
{
    for (int i = 0; i < a.length(); i++)
    {
        if (isdigit(a[i]) == 0)
            return false;
    }
    return true;
}

static int	ft_intlen(int n)
{
	int	ret;

	ret = 1;
	while (n >= 10)
	{
		ret++;
		n = n / 10;
	}
	return (ret);
}

static std::string ft_make_ret(int n, int sign)
{
	int		len;

	len = ft_intlen(n) + sign;
	len--;
    std::string ret(len + 1, '\0');
	while (len >= 0)
	{
		ret[len] = (n % 10) + '0';
		n = n / 10;
		len--;
	}
	if (sign == 1)
		ret[0] = '-';
	return (ret);
}

std::string	ft_itoa(int n)
{
    std::string ret;
	int		sign;

	sign = 0;
	if (n == -2147483648)
	{
		ret = "-2147483648";
		return (ret);
	}
	if (n < 0)
	{
		n *= -1;
		sign = 1;
	}
	return (ft_make_ret(n, sign));
}

class db
{
public:
    db() {}

    db(std::string name, std::vector<std::string> initial_values)
        :name_(name), header(initial_values)
    {}

    void rename(std::string new_name)
    {
        name_ = new_name;
    }
    
    int add_value(std::vector<std::string> new_value)
    {
        values.push_back(new_value);
        for (int i = 0; i < header.size(); i++)
        {
            if (i > new_value.size() - 1)
                break;
            if (querying_cache.contains(header[i]) == true) 
            {
                if (querying_cache[header[i]].contains(new_value[i]) == true)
                {
                    if (overrides.contains(new_value[i]) == true)
                    {
                        int x = overrides[new_value[i]] + 1;
                        overrides[new_value[i]] = x;
                        //std::cout << "A" << std::endl;
                    }
                    else
                        overrides.insert({new_value[i], 1});
                    //std::cout << overrides[new_value[i]] << " " << new_value[i] << std::endl;
                    new_value[i].append(ft_itoa(overrides[new_value[i]]));
                }
                querying_cache[header[i]].insert({new_value[i], {i, size}});   
            }
            else
            {
                querying_cache.insert({header[i], {}});
                if (querying_cache[header[i]].contains(new_value[i]) == true)
                {
                    if (overrides.contains(new_value[i]) == true)
                    {
                        int x = overrides[new_value[i]] + 1;
                        overrides[new_value[i]] = x;
                    }
                    else
                        overrides.insert({new_value[i], 1});
                    std::cout << overrides[new_value[i]] << " " << new_value[i] << std::endl;
                    new_value[i].append(ft_itoa(overrides[new_value[i]]));
                }
                querying_cache[header[i]].insert({new_value[i], {i, size}});
            }
        }
        size++;
        return size - 1;
    }
    
    std::vector<std::vector<std::string>> search(std::string column, std::string value)
    {
        std::vector<std::vector<std::string>> ret;
        if (querying_cache.contains(column) == true)
        {
            if (querying_cache[column].contains(value) == true)
            {
                std::vector<int> pos = querying_cache[column][value];
                std::vector<std::string> add_ve = values[pos[1]];
                ret.push_back(add_ve);
                if (overrides.contains(value) == true)
                {
                    int loops = overrides[value];
                    std::string initial_value = value;
                    for (int i = 1; i <= loops; i++)
                    {
                        value = std::format("{}{}", initial_value, ft_itoa(i));
                        std::vector<int> posi = querying_cache[column][value];
                        std::vector<std::string> add_vec = values[posi[1]];
                        ret.push_back(add_vec);
                    }
                }
                return ret;
            }
        }
        return ret;
    }

    std::unordered_map<std::string, std::vector<int>> get_column(std::string column)
    {
        if (querying_cache.contains(column) == true)
        {
            return querying_cache[column];
        }
        std::unordered_map<std::string, std::vector<int>> ret;
        return ret;
    }

    void column_names()
    {
        for (auto& [key, value]: querying_cache)
        {
            std::cout << key << std::endl;
        }
    }

    bool exists(std::string column, std::string value)
    {
        if (querying_cache.contains(column) == true)
        {
            if (querying_cache[column].contains(value) == true)
            {
                return true;
            }
        }
        return false;
    }

    void remove_value(int index)
    {
        values.erase(values.begin() + index);
        size--;
    }

    std::vector<std::vector<std::string>> get_values()
    {
        return values;
    }

    std::vector<std::string> get_value(int index)
    {
        return values[index];
    }

    std::string get_item(int row, int column)
    {
        return values[row][column];
    }

    std::string get_name()
    {
        return name_;
    }

    int get_size()
    {
        return size;
    }

    int header_size()
    {
        return header.size();
    }
    std::vector<std::string> get_header()
    {
        return header;
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> cache()
    {
        return querying_cache;
    }

    

private:
    std::string name_;
    std::vector<std::string> header;
    std::vector<std::vector<std::string>> values;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<int>>> querying_cache;
    std::map<std::string, int> overrides;
    int size = 0;
};
