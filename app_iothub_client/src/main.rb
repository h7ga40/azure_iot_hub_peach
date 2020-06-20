
connectionString = AzureIoT.get_connection_string
protocol = AzureIoT::MQTT

module WeatherStation
	class ContosoAnemometer
		attr_reader :windSpeed, :temperature, :humidity

		def quit(peyload)
			puts "execute quit " + peyload
		end

		def turnFanOn(peyload)
			puts "execute turnFanOn " + peyload
		end

		def turnFanOff(peyload)
			puts "execute turnFanOff " + peyload
		end

		def set_fan_speed(fanSpeed)
			@fanSpeed = fanSpeed
			puts "set fan speed " + fanSpeed.to_s
		end

		def recv_twin(peyload)
			json = JSON.parse(peyload)
			desired = json["desired"]
			if desired == nil
				desired = json
			end
			fanSpeed = desired["fanSpeed"]
			if fanSpeed == nil
				puts peyload
			else
				value = fanSpeed["value"]
				if value == nil
					puts peyload
				else
					set_fan_speed(value)
				end
			end

			{fanSpeed: {value: @fanSpeed, status: "success"}}.to_json
		end

		def measure
			@windSpeed = 10 + (rand() * 4) + 2
			@temperature = 20 + (rand() * 15)
			@humidity = 60 + (rand() * 20)
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

