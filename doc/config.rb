class Deplate::Core
	def self.user_setup(options)
		options.disabled_particles << Deplate::HyperLink::Simple
	end
end
