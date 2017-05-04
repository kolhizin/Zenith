#pragma once

#include <map>
#include <list>
#include <string>
#include <memory>
#include <set>

namespace zenith
{
	namespace util
	{
		class ObjectLayout;

		class ObjectLayoutField
		{
			std::weak_ptr<ObjectLayout> complexTypeLayoutPtr_;
		public:
			enum class BaseType { UNDEF, DOUBLE, INT, STRING, COMPLEX };
			enum class SetType { UNDEF, SINGLE, MULTIPLE };
		
			BaseType baseType = BaseType::UNDEF;
			SetType setType = SetType::UNDEF;
			bool isOptional = false, isAttribute = false;
			std::list<std::string> aliases;

			bool isValid() const
			{
				if (baseType == BaseType::UNDEF)
					return false;
				if (setType == SetType::UNDEF)
					return false;
				if (aliases.size() == 0)
					return false;
				return true;
			}
		};

		class ObjectLayout
		{
		public:
			enum class BaseType { NONE, DOUBLE, INT, STRING, COMPLEX };
			enum class SetType { NONE, SINGLE, LIST, MAP };

			
		private:
			bool finalized_;			
			std::set<std::string> usedAliases_;

			std::string name;
			std::map<std::string, ObjectLayoutField> fields;
		public:

		};

		class ObjectLayoutManager
		{
			std::map<std::string, std::shared_ptr<ObjectLayout>> layouts_;
		public:
			std::weak_ptr<ObjectLayout> registerLayout(const ObjectLayout &ol);
			std::weak_ptr<ObjectLayout> requestLayout(const std::string &name);
		};
	}
}