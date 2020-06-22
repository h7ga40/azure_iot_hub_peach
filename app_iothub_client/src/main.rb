
connectionString = AzureIoT.get_connection_string
protocol = AzureIoT::MQTT

module WeatherStation
	class ContosoAnemometer
		attr_reader :windSpeed, :temperature, :humidity

		def initialize
			@threshold = 28
		end

		def quit(peyload)
			puts "execute quit " + peyload
			"{\"Message\":\"quit with Method\"}"
		end

		def turnLedOn(peyload)
			puts "execute turnLedOn " + peyload
			PinKit.led = true
			"{\"Message\":\"Turning LED on with Method\"}"
		end

		def turnLedOff(peyload)
			puts "execute turnLedOff " + peyload
			PinKit.led = false
			"{\"Message\":\"Turning LED off with Method\"}"
		end

		def set_threshold(threshold)
			@threshold = threshold
			puts "set fan speed " + threshold.to_s
		end

		def recv_twin(peyload)
			json = JSON.parse(peyload)
			desired = json["desired"]
			if desired == nil
				desired = json
			end
			threshold = desired["threshold"]
			if threshold == nil
				puts peyload
			else
				value = threshold["value"]
				if value == nil
					puts peyload
				else
					set_threshold(value)
				end
			end

			{threshold: {value: @threshold, status: "success"}}.to_json
		end

		def measure
			@windSpeed = PinKit.wind_speed
			@temperature = PinKit.temperature
			@humidity = PinKit.humidity
		end
	end
end

twin = WeatherStation::ContosoAnemometer.new

client = AzureIoT::DeviceClient.new(connectionString, protocol)
client.set_twin(twin)

while true do
	twin.measure
	data = {temperature: twin.temperature, humidity: twin.humidity}.to_json
	message = AzureIoT::Message.new(data)

	message.add_property('temperatureAlert', (twin.temperature > 30) ? 'true' : 'false')

	done = false
	client.send_event(message) do
		puts "sent message"
		done = true
	end

	count = 5000
	while !done do
		client.do_work
		sleep(0.001)
		if count > 0 then
			count -= 1
		end
	end

	if count > 0 then
		sleep(0.001 * count)
	end
end

