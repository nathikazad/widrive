require File.expand_path('../../lib/em-websocket', __FILE__)
require 'base64'
image=""
left_count=0
right_count=0
forward_count=0
reverse_count=0
snapshot_count=0
EM.run {
  @mbed_device = EM::Channel.new
  @client = EM::Channel.new
  EM::WebSocket.run(:host => "0.0.0.0", :port => 8080, :debug => false) do |ws|
    puts "WebSocket opened"
    ws.onopen { |handshake|
      puts "WebSocket opened #{{
        :path => handshake.path,
        :origin => handshake.origin,
      }}"
      if(handshake.path=="/mbed")
        ws.send "Hello mbed!"
        puts "Mbed Device Connected"
        sid = @mbed_device.subscribe { |msg| ws.send msg }
        ws.onmessage { |msg|
          if msg=="finished_sending"
            @client.push image
            puts "mbed to client:finished sending image"
            image=""
          elsif msg[0..3]=="img:"
            image=image+msg[4..-1]
            puts "mbed to client:got chunk #{msg}"
          else
            @client.push msg
            puts "mbed to client:#{msg}"
          end
        }
        ws.onclose {
          @mbed_device.unsubscribe(sid)
          puts "Mbed Device Disconnected"
        }
      elsif(handshake.path=="/client")
        ws.send "Hello client!"
        ws.send "stats:#{left_count},#{right_count},#{forward_count},#{reverse_count},#{snapshot_count}"
        puts "Client Device Connected"
        sid = @client.subscribe { |msg| ws.send msg }
        ws.onmessage { |msg|
          @mbed_device.push msg
          puts "client to mbed:#{msg}"
          if msg=="move_forward"
            forward_count+=1
            @client.push "stats:#{left_count},#{right_count},#{forward_count},#{reverse_count},#{snapshot_count}"
          elsif msg=="turn_right"
            right_count+=1
            @client.push "stats:#{left_count},#{right_count},#{forward_count},#{reverse_count},#{snapshot_count}"
          elsif msg=="turn_left"
            left_count+=1
            @client.push "stats:#{left_count},#{right_count},#{forward_count},#{reverse_count},#{snapshot_count}"
          elsif msg=="move_backward"
            reverse_count+=1
            @client.push "stats:#{left_count},#{right_count},#{forward_count},#{reverse_count},#{snapshot_count}"
          elsif msg=="snap_image"
            snapshot_count+=1
            @client.push "stats:#{left_count},#{right_count},#{forward_count},#{reverse_count},#{snapshot_count}"
          end
        }
        ws.onclose {
          @client.unsubscribe(sid)
          puts "Client Device Disconnected"
        }
      end
      ws.onerror { |e|
        puts "Error: #{e.message}"
      }
    }
  end
}
