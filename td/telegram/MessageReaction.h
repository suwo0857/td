//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2023
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "td/telegram/ChannelId.h"
#include "td/telegram/DialogId.h"
#include "td/telegram/FullMessageId.h"
#include "td/telegram/MessageId.h"
#include "td/telegram/MinChannel.h"
#include "td/telegram/ReactionType.h"
#include "td/telegram/td_api.h"
#include "td/telegram/telegram_api.h"
#include "td/telegram/UserId.h"

#include "td/utils/common.h"
#include "td/utils/FlatHashMap.h"
#include "td/utils/Promise.h"
#include "td/utils/StringBuilder.h"

#include <utility>

namespace td {

class Dependencies;

class Td;

class MessageReaction {
  static constexpr int32 MAX_CHOOSE_COUNT = 2147483640;

  static constexpr size_t MAX_RECENT_CHOOSERS = 3;

  ReactionType reaction_type_;
  int32 choose_count_ = 0;
  bool is_chosen_ = false;
  DialogId my_recent_chooser_dialog_id_;
  vector<DialogId> recent_chooser_dialog_ids_;
  vector<std::pair<ChannelId, MinChannel>> recent_chooser_min_channels_;

  friend bool operator==(const MessageReaction &lhs, const MessageReaction &rhs);

  friend StringBuilder &operator<<(StringBuilder &string_builder, const MessageReaction &message_reaction);

  friend struct MessageReactions;

  MessageReaction(ReactionType reaction_type, int32 choose_count, bool is_chosen, DialogId my_recent_chooser_dialog_id,
                  vector<DialogId> &&recent_chooser_dialog_ids,
                  vector<std::pair<ChannelId, MinChannel>> &&recent_chooser_min_channels);

  bool is_empty() const {
    return choose_count_ <= 0;
  }

  bool is_chosen() const {
    return is_chosen_;
  }

  void set_as_chosen(DialogId my_dialog_id, bool have_recent_choosers);

  void unset_as_chosen();

  void add_recent_chooser_dialog_id(DialogId dialog_id);

  bool remove_recent_chooser_dialog_id();

  void update_from(const MessageReaction &old_reaction);

  void update_recent_chooser_dialog_ids(const MessageReaction &old_reaction);

  int32 get_choose_count() const {
    return choose_count_;
  }

  void set_my_recent_chooser_dialog_id(DialogId my_dialog_id);

  DialogId get_my_recent_chooser_dialog_id() const {
    return my_recent_chooser_dialog_id_;
  }

  const vector<DialogId> &get_recent_chooser_dialog_ids() const {
    return recent_chooser_dialog_ids_;
  }

  const vector<std::pair<ChannelId, MinChannel>> &get_recent_chooser_min_channels() const {
    return recent_chooser_min_channels_;
  }

  td_api::object_ptr<td_api::messageReaction> get_message_reaction_object(Td *td, UserId my_user_id,
                                                                          UserId peer_user_id) const;

 public:
  MessageReaction() = default;

  const ReactionType &get_reaction_type() const {
    return reaction_type_;
  }

  template <class StorerT>
  void store(StorerT &storer) const;

  template <class ParserT>
  void parse(ParserT &parser);
};

bool operator==(const MessageReaction &lhs, const MessageReaction &rhs);

inline bool operator!=(const MessageReaction &lhs, const MessageReaction &rhs) {
  return !(lhs == rhs);
}

StringBuilder &operator<<(StringBuilder &string_builder, const MessageReaction &reaction);

class UnreadMessageReaction {
  ReactionType reaction_type_;
  DialogId sender_dialog_id_;
  bool is_big_ = false;

  friend bool operator==(const UnreadMessageReaction &lhs, const UnreadMessageReaction &rhs);

  friend StringBuilder &operator<<(StringBuilder &string_builder, const UnreadMessageReaction &message_reaction);

 public:
  UnreadMessageReaction() = default;

  UnreadMessageReaction(ReactionType reaction_type, DialogId sender_dialog_id, bool is_big)
      : reaction_type_(std::move(reaction_type)), sender_dialog_id_(sender_dialog_id), is_big_(is_big) {
  }

  td_api::object_ptr<td_api::unreadReaction> get_unread_reaction_object(Td *td) const;

  template <class StorerT>
  void store(StorerT &storer) const;

  template <class ParserT>
  void parse(ParserT &parser);
};

bool operator==(const UnreadMessageReaction &lhs, const UnreadMessageReaction &rhs);

inline bool operator!=(const UnreadMessageReaction &lhs, const UnreadMessageReaction &rhs) {
  return !(lhs == rhs);
}

StringBuilder &operator<<(StringBuilder &string_builder, const UnreadMessageReaction &unread_reaction);

struct MessageReactions {
  vector<MessageReaction> reactions_;
  vector<UnreadMessageReaction> unread_reactions_;
  vector<ReactionType> chosen_reaction_order_;
  bool is_min_ = false;
  bool need_polling_ = true;
  bool can_get_added_reactions_ = false;

  MessageReactions() = default;

  static unique_ptr<MessageReactions> get_message_reactions(Td *td,
                                                            tl_object_ptr<telegram_api::messageReactions> &&reactions,
                                                            bool is_bot);

  MessageReaction *get_reaction(const ReactionType &reaction_type);

  const MessageReaction *get_reaction(const ReactionType &reaction_type) const;

  void update_from(const MessageReactions &old_reactions);

  bool add_reaction(const ReactionType &reaction_type, bool is_big, DialogId my_dialog_id, bool have_recent_choosers);

  bool remove_reaction(const ReactionType &reaction_type, DialogId my_dialog_id);

  void sort_reactions(const FlatHashMap<ReactionType, size_t, ReactionTypeHash> &active_reaction_pos);

  void fix_chosen_reaction();

  void fix_my_recent_chooser_dialog_id(DialogId my_dialog_id);

  vector<ReactionType> get_chosen_reaction_types() const;

  bool are_consistent_with_list(const ReactionType &reaction_type,
                                FlatHashMap<ReactionType, vector<DialogId>, ReactionTypeHash> reaction_types,
                                int32 total_count) const;

  vector<td_api::object_ptr<td_api::messageReaction>> get_message_reactions_object(Td *td, UserId my_user_id,
                                                                                   UserId peer_user_id) const;

  void add_min_channels(Td *td) const;

  void add_dependencies(Dependencies &dependencies) const;

  static bool need_update_message_reactions(const MessageReactions *old_reactions,
                                            const MessageReactions *new_reactions);

  static bool need_update_unread_reactions(const MessageReactions *old_reactions,
                                           const MessageReactions *new_reactions);

  template <class StorerT>
  void store(StorerT &storer) const;

  template <class ParserT>
  void parse(ParserT &parser);

 private:
  bool do_remove_reaction(const ReactionType &reaction_type);
};

StringBuilder &operator<<(StringBuilder &string_builder, const MessageReactions &reactions);

StringBuilder &operator<<(StringBuilder &string_builder, const unique_ptr<MessageReactions> &reactions);

void reload_message_reactions(Td *td, DialogId dialog_id, vector<MessageId> &&message_ids);

void send_message_reaction(Td *td, FullMessageId full_message_id, vector<ReactionType> reaction_types, bool is_big,
                           bool add_to_recent, Promise<Unit> &&promise);

void get_message_added_reactions(Td *td, FullMessageId full_message_id, ReactionType reaction_type, string offset,
                                 int32 limit, Promise<td_api::object_ptr<td_api::addedReactions>> &&promise);

void report_message_reactions(Td *td, FullMessageId full_message_id, DialogId chooser_dialog_id,
                              Promise<Unit> &&promise);

}  // namespace td
