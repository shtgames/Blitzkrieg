#ifndef INTELLIGENCE_BACKEND
#define INTELLIGENCE_BACKEND

namespace bEnd
{
	class Espionage final
	{
	public:
		const float& getFreeSpies()const { return freeSpies; };

		void update();

		const float setLeadership(const float);

	private:
		float freeSpies;
	};
}

#endif
