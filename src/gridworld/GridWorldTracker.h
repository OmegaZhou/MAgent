#ifndef MAGENT_GRIDWORLD_TRACKER_H
#define MAGENT_GRIDWORLD_TRACKER_H
#include "grid_def.h"
#include <fstream>
#include <string>
#include <set>
#include <map>
namespace magent
{
    namespace gridworld
    {
        class Agent;
        typedef int GroupHandle;
        struct TraceInfo
        {
        public:
            enum DeadReason
            {
                BE_ATTACKED,
                BE_STARVED,
                BE_ABSORBED
            };
            enum AttackAction
            {
                ATTACK_BLANK,
                ATTACK_SUCCESS,
                ATTACK_KILL,
                ATTACK_NULL
            };
            enum StarveAction
            {
                RECORVERY,
                STARVED,
                DEAD,
                STARVE_NULL
            };
            enum TurnAction
            {
                CLOCKWISE,
                ANTICLOCKWISE,
                TURN_FAIL,
                TURN_NULL
            };
            enum MoveAction
            {
                MOVE_SUCCESS,
                MOVE_BE_ABSORBED,
                MOVE_FAIL,
                MOVE_NULL
            };
            void printInfo(std::ostream &os, char separator = ',');
            static void printHead(std::ostream &os, char separator = ',');

            TraceInfo(int time, int agent_id, GroupHandle group_id) : time_stamp(time), agent_id(agent_id), group_id(group_id)
            {
                attack_action = AttackAction::ATTACK_NULL;
                starve_action = StarveAction::STARVE_NULL;
                turn_action = TurnAction::TURN_NULL;
                move_action = MoveAction::MOVE_NULL;
                dead_result = false;
            }
            int time_stamp;
            GroupHandle group_id;
            int agent_id;

            AttackAction attack_action;
            Position attack_goal;
            Reward attack_reward;

            StarveAction starve_action;

            TurnAction turn_action;
            Direction turn_old_dir;
            Position turn_old_position;
            Direction turn_new_dir;
            Position turn_new_position;

            MoveAction move_action;
            Position move_old_position;
            Position move_new_position;

            bool dead_result;
            DeadReason dead_reason;
            int dead_time;
            float hp;
            Reward total_reward;

        private:
            void printPosition(std::ostream &os, const Position &p)
            {
                os << "(" << p.x << "|" << p.y << ")";
            }
            static const std::string direction_str[];
            static const std::string dead_reason_str[];
            static const std::string attack_action_str[];
            static const std::string starve_action_str[];
            static const std::string turn_action_str[];
            static const std::string move_action_str[];
        };
        class TraceTracker
        {
        public:
            TraceTracker()
            {
                outHead();
                time_stamp = 1;
            }
            void newStep()
            {
                ++time_stamp;
            }
            void attack(Agent *agent, Position p, Reward reward, bool is_kill, bool is_blank);

            void starve(Agent *agent, int old_hp);
            void turnWise(Agent *agent, int wise, Position old_pos, Direction old_dir);
            void move(Agent *agent, Position old_position);
            void dead(Agent *agent, TraceInfo::DeadReason reason);
            void reset()
            {
                flush(time_stamp);
                outHead();
                time_stamp = 1;
                dead_id.clear();
                m.clear();
            }
            ~TraceTracker()
            {
                flush(time_stamp);
                
            }

        private:
            int file_count=0;
            std::string now_path_name;
            TraceInfo &getInfo(int t, Agent *agent);
            bool check_dead(int agent_id)
            {
                return dead_id.find(agent_id) != dead_id.end();
            }
            void add_dead(int agent_id)
            {
                dead_id.insert(agent_id);
            }
            void buffer_count()
            {
            }
            void outHead()
            {
                
                os.open(now_path_name, std::ios::app);
                TraceInfo::printHead(os);
                os.close();
            }
            void flush(int t)
            {
                os.open(now_path_name, std::ios::app);
                if (m.empty())
                {
                    return;
                }
                //std::cout<<"I'm running"<<std::endl;
                int st = m.begin()->first;
                for (int i = st; i <= t; ++i)
                {
                    auto iter = m.find(i);
                    if (iter == m.end())
                    {
                        continue;
                    }
                    auto &mp = iter->second;
                    for (auto it = mp.begin(); it != mp.end(); ++it)
                    {
                        it->second.printInfo(os);
                    }
                }
                os.close();
            }
            static bool equal(const Position &p1, const Position &p2)
            {
                return p1.x == p2.x && p1.y == p2.y;
            }
            int time_stamp;
            std::set<int> dead_id;
            std::map<int, std::map<int, TraceInfo>> m;
            const int buffer_size = 100;
            int now_count = 0;
            std::ofstream os;
        };
    } // namespace gridworld

    // namespace gridworld
} // namespace magent
#endif