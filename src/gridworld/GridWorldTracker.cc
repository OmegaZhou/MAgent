#include "GridWorldTracker.h"
#include "GridWorld.h"
namespace magent
{
    namespace gridworld
    {
        void TraceInfo::printInfo(std::ostream &os, char separator)
        {
            os << time_stamp << separator << group_id << separator << agent_id << separator;

            os << TraceInfo::attack_action_str[attack_action] << separator;
            if (attack_action != AttackAction::ATTACK_NULL)
            {
                printPosition(os, attack_goal);
                os << separator;
                os << attack_reward << separator;
            }
            else
            {
                os << separator << separator;
            }

            os << TraceInfo::starve_action_str[starve_action] << separator;
            //std::cout << TraceInfo::starve_action_str[starve_action] << starve_action << std::endl;

            os << TraceInfo::turn_action_str[turn_action] << separator;
            if (turn_action != TurnAction::TURN_NULL)
            {
                os << TraceInfo::direction_str[turn_old_dir] << separator;
                printPosition(os, turn_old_position);
                os << separator;
                os << TraceInfo::direction_str[turn_new_dir] << separator;
                printPosition(os, turn_new_position);
                os << separator;
            }
            else
            {
                os << separator << separator << separator << separator;
            }

            os << TraceInfo::move_action_str[move_action] << separator;
            if (move_action != MoveAction::MOVE_NULL)
            {
                printPosition(os, move_old_position);
                os << separator;
                printPosition(os, move_new_position);
                os << separator;
            }
            else
            {
                os << separator << separator;
            }

            os << dead_result << separator;
            if (dead_result)
            {
                os << TraceInfo::dead_reason_str[dead_reason] << separator << dead_time;
            }
            else
            {
                os << separator;
            }
            os << hp << separator << total_reward;
            os << std::endl;
        }
        void TraceInfo::printHead(std::ostream &os, char separator)
        {
            os << "step" << separator << "group_id" << separator << "agent_id" << separator;
            os << "attack_action" << separator << "attack_goal" << separator << "attack_reward" << separator;
            os << "starve_action" << separator;
            os << "turn_action" << separator << "turn_old_dir" << separator << "turn_old_position" << separator
               << "turn_new_dir" << separator << "turn_new_position" << separator;
            os << "move_action" << separator << "move_old_position" << separator << "move_new_position" << separator;
            os << "is_dead" << separator << "dead_reason" << separator << "dead_time";
            os << "hp" << separator << "total_reward";
            os << std::endl;
        }
        const std::string TraceInfo::direction_str[] = {"EAST", "SOUTH", "WEST", "NORTH"};
        const std::string TraceInfo::dead_reason_str[] = {"BE_ATTACKED", "BE_STARVED", "BE_ABSORBED"};
        const std::string TraceInfo::attack_action_str[] = {"ATTACK_BLANK", "ATTACK_SUCCESS", "ATTACK_KILL", "ATTACK_NULL"};
        const std::string TraceInfo::starve_action_str[] = {"RECORVERY", "STARVED", "DEAD", "STARVE_NULL"};
        const std::string TraceInfo::turn_action_str[] = {"CLOCKWISE", "ANTICLOCKWISE", "TURN_FAIL", "TURN_NULL"};
        const std::string TraceInfo::move_action_str[] = {"MOVE_SUCCESS", "BE_ABSORBED", "MOVE_FAIL", "MOVE_NULL"};
        void TraceTracker::attack(Agent *agent, Position p, Reward reward, bool is_kill, bool is_blank)
        {
            if (check_dead(agent->get_id()))
            {
                return;
            }
            TraceInfo &info = getInfo(time_stamp, agent);
            if (is_kill)
            {
                info.attack_action = TraceInfo::AttackAction::ATTACK_KILL;
            }
            else if (is_blank)
            {
                info.attack_action = TraceInfo::AttackAction::ATTACK_BLANK;
            }
            else
            {
                info.attack_action = TraceInfo::AttackAction::ATTACK_SUCCESS;
            }
            info.attack_goal = p;
            info.attack_reward = reward;
            updateBaseInfo(info, agent);
        }
        void TraceTracker::starve(Agent *agent, int old_hp)
        {
            if (check_dead(agent->get_id()))
            {
                return;
            }
            TraceInfo &info = getInfo(time_stamp, agent);
            if (agent->is_dead())
            {
                dead(agent, TraceInfo::DeadReason::BE_STARVED);
                info.starve_action = TraceInfo::StarveAction::DEAD;
            }
            else if (agent->get_hp() > old_hp)
            {
                info.starve_action = TraceInfo::StarveAction::RECORVERY;
            }
            else if (agent->get_hp() < old_hp)
            {
                info.starve_action = TraceInfo::StarveAction::STARVED;
            }
            else
            {
                info.starve_action = TraceInfo::StarveAction::STARVE_NULL;
            }
            updateBaseInfo(info, agent);
        }
        void TraceTracker::turnWise(Agent *agent, int wise, Position old_pos, Direction old_dir)
        {
            if (check_dead(agent->get_id()))
            {
                return;
            }
            TraceInfo &info = getInfo(time_stamp, agent);
            Direction dir = agent->get_dir();
            if (dir == old_dir && equal(old_pos, agent->get_pos()))
            {
                info.turn_action = TraceInfo::TurnAction::TURN_FAIL;
            }
            else
            {
                if (wise == 1)
                {
                    info.turn_action = TraceInfo::TurnAction::CLOCKWISE;
                }
                else
                {
                    info.turn_action = TraceInfo::TurnAction::ANTICLOCKWISE;
                }
            }
            info.turn_old_dir = old_dir;
            info.turn_old_position = old_pos;
            info.turn_new_dir = dir;
            info.turn_new_position = agent->get_pos();
            updateBaseInfo(info, agent);
        }
        void TraceTracker::move(Agent *agent, Position old_position)
        {
            if (check_dead(agent->get_id()))
            {
                return;
            }
            TraceInfo &info = getInfo(time_stamp, agent);
            info.move_new_position = agent->get_pos();
            info.move_old_position = old_position;
            if (agent->is_dead())
            {
                info.move_action = TraceInfo::MoveAction::MOVE_BE_ABSORBED;
                dead(agent, TraceInfo::DeadReason::BE_ABSORBED);
            }
            else if (equal(old_position, agent->get_pos()))
            {
                info.move_action = TraceInfo::MoveAction::MOVE_FAIL;
            }
            else
            {
                info.move_action = TraceInfo::MoveAction::MOVE_SUCCESS;
            }
            updateBaseInfo(info, agent);
        }
        void TraceTracker::dead(Agent *agent, TraceInfo::DeadReason reason)
        {
            if (check_dead(agent->get_id()))
            {
                return;
            }
            TraceInfo &info = getInfo(time_stamp, agent);
            dead_id.insert(agent->get_id());
            info.dead_result = true;
            info.dead_time = time_stamp;
            info.dead_reason = reason;
            updateBaseInfo(info, agent);
        }
        void updateBaseInfo(TraceInfo &info, Agent *agent)
        {
            info.hp = agent->get_hp();
            info.total_reward = agent->get_reward();
        }
        TraceInfo &TraceTracker::getInfo(int t, Agent *agent)
        {
            int agent_id = agent->get_id();
            //std::cout<<"test"<<std::endl;
            GroupHandle group_id = agent->get_group();
            auto time_iter = m.find(t);
            if (time_iter == m.end())
            {
                m.insert(std::make_pair(t, std::map<int, TraceInfo>()));
                time_iter = m.find(t);
            }
            std::map<int, TraceInfo> &trace_m = time_iter->second;
            auto info_iter = trace_m.find(agent_id);
            if (info_iter == trace_m.end())
            {
                trace_m.insert(std::make_pair(agent_id, TraceInfo(t, agent_id, group_id)));
                info_iter = trace_m.find(agent_id);
            }
            return info_iter->second;
        }

    } // namespace gridworld
} // namespace magent
