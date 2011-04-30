#ifndef VERIFYTAPN_TAPN_TIMEINVARIANT_HPP_
#define VERIFYTAPN_TAPN_TIMEINVARIANT_HPP_

#include <string>
#include <limits>

namespace VerifyTAPN {
	namespace TAPN {

		class TimeInvariant
		{
		public:
			static const TimeInvariant LS_INF;

		public: // construction/destruction
			TimeInvariant() : strictComparison(true), bound(std::numeric_limits<int>::max()) { };
			TimeInvariant(bool strictComparison, int bound) : strictComparison(strictComparison), bound(bound) { };
			TimeInvariant(const TimeInvariant& ti) : strictComparison(ti.strictComparison), bound(ti.bound) { };
			TimeInvariant& operator=(const TimeInvariant& ti)
			{
				strictComparison = ti.strictComparison;
				bound = ti.bound;
				return *this;
			}

			virtual ~TimeInvariant() { /* Empty */ };
		public: // inspectors
			void Print(std::ostream& out) const;
			inline const int GetBound() const { return bound; }
			inline const bool IsBoundStrict() const { return strictComparison; }

		public: // statics
			static TimeInvariant CreateFor(const std::string& invariant);

		private: // data
			bool strictComparison;
			int bound;
		};

		inline bool operator==(const TimeInvariant& a, const TimeInvariant& b)
		{
			return a.GetBound() == b.GetBound() && a.IsBoundStrict() == b.IsBoundStrict();
		}

		inline bool operator!=(const TimeInvariant& a, const TimeInvariant& b)
		{
			return !(a == b);
		}

		inline std::ostream& operator<<(std::ostream& out, const TimeInvariant& invariant)
		{
			invariant.Print(out);
			return out;
		}
	}

}

#endif /* VERIFYTAPN_TAPN_TIMEINVARIANT_HPP_ */
