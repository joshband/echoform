#pragma once

namespace ui_runtime {

class StateMachine {
public:
    using StateId = int;

    constexpr StateMachine(StateId initialState = 0) noexcept
        : state_(initialState)
    {
    }

    void setState(StateId nextState) noexcept { state_ = nextState; }
    StateId getState() const noexcept { return state_; }

private:
    StateId state_ { 0 };
};

} // namespace ui_runtime
