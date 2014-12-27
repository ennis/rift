#ifndef RENDERQUEUE_HPP
#define RENDERQUEUE_HPP

#include <renderer.hpp>
#include <submission.hpp>
#include <vector>

// class RenderQueue
// Gère les listes de Submissions (tri)
class RenderQueue
{
public:
	// default-constructible / nullable
	RenderQueue() = default;
	// noncopyable
	RenderQueue(const RenderQueue&) = delete;
	RenderQueue &operator=(const RenderQueue &) = delete;
	// move-constructible
	RenderQueue(RenderQueue &&rhs);
	// moveable
	RenderQueue &operator=(RenderQueue &&rhs);

	// add a submission
	void addSubmission(const Submission &submission);
	// clear the list of submissions (without rendering them)
	void clear();
	// get the list of submissions
	const std::vector<Submission> &getSubmissions() const; 

private:
	std::vector<Submission> mSubmissions;
};

 
#endif /* end of include guard: RENDERQUEUE_HPP */