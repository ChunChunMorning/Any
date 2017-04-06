# pragma once
# include <typeinfo>

class bad_any_cast : public std::bad_cast
{
public:

	virtual ~bad_any_cast() = default;

	virtual const char* what() const
	{
		return "bad any cast";
	}
};

class Any
{
private:

	class PlaceHolder
	{
	public:

		virtual ~PlaceHolder() = default;

		virtual const std::type_info& type() const = 0;

		virtual PlaceHolder* clone() const = 0;
	};

	template <class ValueType>
	class Holder : public PlaceHolder
	{
	private:

		ValueType m_value;

	public:

		Holder(const ValueType& value) : m_value(value) {}

		Holder(ValueType&& value) : m_value(std::move(value)) {}

		template <class ... Args >
		Holder(Args&& ... args) : m_value(ValueType(args...)) {}

		ValueType& get()
		{
			return m_value;
		}

		const std::type_info& type() const override
		{
			return typeid(ValueType);
		}

		PlaceHolder* clone() const override
		{
			return new Holder(m_value);
		}
	};

	PlaceHolder* m_value;

public:

	Any() : m_value(nullptr) {}

	template <class ValueType>
	Any(const ValueType& other) : m_value(new Holder<ValueType>(other)) {}

	Any(const Any& other) : m_value(nullptr)
	{
		if (other.has_value())
		{
			m_value = other.m_value->clone();
		}
	}

	Any(Any&& other) : m_value(nullptr)
	{
		if (other.has_value())
		{
			m_value = other.m_value;
			other.m_value = nullptr;
		}
	}

	virtual ~Any()
	{
		if (has_value())
		{
			delete m_value;
		}
	}

	Any& operator = (const Any& rhs)
	{
		if (this != &rhs)
		{
			if (has_value())
			{
				delete m_value;
				m_value = nullptr;
			}

			if (rhs.has_value())
			{
				m_value = rhs.m_value->clone();
			}
		}

		return *this;
	}

	Any& operator = (Any&& rhs)
	{
		if (this != &rhs)
		{
			if (has_value())
			{
				delete m_value;
				m_value = nullptr;
			}

			if (rhs.has_value())
			{
				m_value = rhs.m_value;
				rhs.m_value = nullptr;
			}
		}

		return *this;
	}

	template <class ValueType>
	Any& operator = (const ValueType& rhs)
	{
		if (has_value())
		{
			delete m_value;
		}

		m_value = new Holder<ValueType>(rhs);
		return *this;
	}

	template <class ValueType>
	Any& operator = (ValueType&& rhs)
	{
		if (has_value())
		{
			delete m_value;
		}

		m_value = new Holder<ValueType>(std::move(rhs));
		return *this;
	}

	template <class ValueType, class ... Args>
	void emplace(Args&& ... args)
	{
		if (has_value())
		{
			delete m_value;
		}

		m_value = new Holder<ValueType>(std::forward<Args>(args)...);
	}

	void reset()
	{
		if (has_value())
		{
			delete m_value;
			m_value = nullptr;
		}
	}

	void swap(Any& other)
	{
		const auto tmp = m_value;
		m_value = other.m_value;
		other.m_value = tmp;
	}

	bool has_value() const
	{
		return m_value != nullptr;
	}

	const std::type_info& type() const
	{
		return m_value->type();
	}

	template <class ValueType>
	friend ValueType& any_cast(const Any& value)
	{
		if (value.type() != typeid(ValueType))
		{
			throw bad_any_cast();
		}

		return static_cast<Holder<ValueType>*>(value.m_value)->get();
	}
};

template <class ValueType, class ... Args>
Any make_any(Args&& ... args)
{
	Any value;
	value.emplace<ValueType>(args...);
	return value;
}
