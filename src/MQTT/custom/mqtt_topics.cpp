#include <MQTT/custom/mqtt_topics.h>
#include <MQTT/ClientID.h>

// Dictionary of subscribed topics (incoming topics for ESP32)
Dictionary<String, String> subscribedTopics = Dictionary<String, String>();

// Dictionary of published topics (outgoing topics from ESP32)
Dictionary<String, String> publishedTopics = Dictionary<String, String>();

static String gTopicPrefix = kDefaultMqttClientId;

void setTopicPrefix(const char *topicPrefix)
{
  if (topicPrefix != NULL && topicPrefix[0] != '\0')
  {
    gTopicPrefix = topicPrefix;
  }
}

static String prefixedTopic(const char *suffix)
{
  return gTopicPrefix + suffix;
}

// builds the subscribed topics dictionary (customizable)
void compileSubTopics(Dictionary<String, String> &subTopics) {
  // subscribed topic di comando del guadagno del PGA
  subTopics.set("pgaSetGainTopic", prefixedTopic("/pgaSetGain"));
  // subscried topic di comando della MSF (one-shot, free-run, stop)
  subTopics.set("triggerTopic", prefixedTopic("/trigger"));
  // subscribed topic for message printing
  subTopics.set("inTopic", prefixedTopic("/inTopic"));
}

// builds the published topics dictionary (customizable)
void compilePubTopics(Dictionary<String, String> &pubTopics) {
  // topic for accelerometer spectrum publication 0
  pubTopics.set("outTopic0", prefixedTopic("/FFTBinTopic0"));
  // topic for accelerometer spectrum publication 1
  pubTopics.set("outTopic1", prefixedTopic("/FFTBinTopic1"));
  // topic for RTD1 value publication
  pubTopics.set("outTopic2", prefixedTopic("/RTD1BinTopic"));
  // topic for RTD1 status publication
  pubTopics.set("outTopic3", prefixedTopic("/RTD1FaultTopic"));
  // topic for RTD2 value publication
  pubTopics.set("outTopic4", prefixedTopic("/RTD2BinTopic"));
  // topic for RTD2 status publication
  pubTopics.set("outTopic5", prefixedTopic("/RTD2FaultTopic"));
  // topic for ADC torque/speed sample counter publication
  pubTopics.set("outTopic6", prefixedTopic("/CountAdcTopic"));
  // topic for torque/speed value publication
  pubTopics.set("outTopic7", prefixedTopic("/TorqueSpeedTopic"));
  // topic for current PGA gain value publication
  pubTopics.set("pgaGetGainTopic", prefixedTopic("/pgaGetGain"));
  // topic for current AUX_DIN pin value publication
  pubTopics.set("auxdinTopic", prefixedTopic("/auxdin"));
}