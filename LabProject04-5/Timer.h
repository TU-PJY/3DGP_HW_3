#pragma once
const ULONG MAX_SAMPLE_COUNT = 50; // Maximum frame time sample count


class Timer{
private:
	double							TimeScale;
	float							ElapsedTime;

	__int64							BasePerformanceCounter;
	__int64							PausedPerformanceCounter;
	__int64							StopPerformanceCounter;
	__int64							CurrentPerformanceCounter;
	__int64							LastPerformanceCounter;

	__int64							PerformanceFrequencyPerSec;

	float							FrameTime[MAX_SAMPLE_COUNT];
	ULONG							SampleCount;

	unsigned long					CurrentFrameRate;
	unsigned long					FramesPerSecond;
	float							FPSTimeElapsed;

	bool							Stopped;


public:
	Timer::Timer() {
		::QueryPerformanceFrequency((LARGE_INTEGER*)&PerformanceFrequencyPerSec);
		::QueryPerformanceCounter((LARGE_INTEGER*)&LastPerformanceCounter);
		TimeScale = 1.0 / (double)PerformanceFrequencyPerSec;

		BasePerformanceCounter = LastPerformanceCounter;
		PausedPerformanceCounter = 0;
		StopPerformanceCounter = 0;

		SampleCount = 0;
		CurrentFrameRate = 0;
		FramesPerSecond = 0;
		FPSTimeElapsed = 0.0f;
	}


	virtual Timer::~Timer() {}


	void Timer::Tick(float LockFPS) {
		if (Stopped) {
			ElapsedTime = 0.0f;
			return;
		}

		float DeltaTime;

		::QueryPerformanceCounter((LARGE_INTEGER*)&CurrentPerformanceCounter);
		DeltaTime = float((CurrentPerformanceCounter - LastPerformanceCounter) * TimeScale);

		if (LockFPS > 0.0f) {
			while (DeltaTime < (1.0f / LockFPS)) {
				::QueryPerformanceCounter((LARGE_INTEGER*)&CurrentPerformanceCounter);
				DeltaTime = float((CurrentPerformanceCounter - LastPerformanceCounter) * TimeScale);
			}
		}

		LastPerformanceCounter = CurrentPerformanceCounter;

		if (fabsf(DeltaTime - ElapsedTime) < 1.0f) {
			::memmove(&FrameTime[1], FrameTime, (MAX_SAMPLE_COUNT - 1) * sizeof(float));
			FrameTime[0] = DeltaTime;

			if (SampleCount < MAX_SAMPLE_COUNT) 
				SampleCount++;
		}

		FramesPerSecond++;
		FPSTimeElapsed += DeltaTime;

		if (FPSTimeElapsed > 1.0f) {
			CurrentFrameRate = FramesPerSecond;
			FramesPerSecond = 0;
			FPSTimeElapsed = 0.0f;
		}

		ElapsedTime = 0.0f;

		for (ULONG i = 0; i < SampleCount; i++)
			ElapsedTime += FrameTime[i];

		if (SampleCount > 0)
			ElapsedTime /= SampleCount;
	}


	unsigned long Timer::GetFrameRate(LPTSTR String, int Characters) {
		if (String) {
			_itow_s(CurrentFrameRate, String, Characters, 10);
			wcscat_s(String, Characters, _T(" FPS)"));
		}

		return(CurrentFrameRate);
	}


	float Timer::GetTimeElapsed() {
		return(ElapsedTime);
	}


	float Timer::GetTotalTime() {
		if (Stopped)
			return(float(((StopPerformanceCounter - PausedPerformanceCounter) - BasePerformanceCounter) * TimeScale));

		return(float(((CurrentPerformanceCounter - PausedPerformanceCounter) - BasePerformanceCounter) * TimeScale));
	}


	void Timer::Reset() {
		__int64 PerformanceCounter;
		::QueryPerformanceCounter((LARGE_INTEGER*)&PerformanceCounter);

		BasePerformanceCounter = PerformanceCounter;
		LastPerformanceCounter = PerformanceCounter;
		StopPerformanceCounter = 0;
		Stopped = false;
	}


	void Timer::Start() {
		__int64 PerformanceCounter;
		::QueryPerformanceCounter((LARGE_INTEGER*)&PerformanceCounter);

		if (Stopped) {
			PausedPerformanceCounter += (PerformanceCounter - StopPerformanceCounter);
			LastPerformanceCounter = PerformanceCounter;
			StopPerformanceCounter = 0;
			Stopped = false;
		}
	}


	void Timer::Stop() {
		if (!Stopped) {
			::QueryPerformanceCounter((LARGE_INTEGER*)&StopPerformanceCounter);
			Stopped = true;
		}
	}
};
